#include "audio/wasapi/wasapi_driver.h"

#include "math/funcs.h"


InternalAudio::AudioAdapter WASAPIDriver::get_vtable()
{
    return InternalAudio::AudioAdapter
    {
        .initialize = &WASAPIDriver::initialize,
        .shutdown = &WASAPIDriver::shutdown,
        .output_get_samples_per_sec = &WASAPIDriver::output_get_samples_per_sec,
        .output_start = &WASAPIDriver::output_start,
        .output_stop = &WASAPIDriver::output_stop,
        .output_wait_for_event = &WASAPIDriver::output_wait_for_event,
        .output_get_frame_count = &WASAPIDriver::output_get_frame_count,
        .output_send_frames = &WASAPIDriver::output_send_frames,
    };
}

void WASAPIDriver::initialize(mem::Allocator* allocator)
{
    data.allocator = allocator;
    WASAPIDebugInfo("Initializing WASAPI Driver...");

    IMMDeviceEnumerator* enumerator = nullptr;

    CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator)
    );

    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &data.output_device.device);
    enumerator->Release();

    data.output_device.device->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL,
        nullptr, reinterpret_cast<void**>(&data.output_device.audio_client)
    );
    
    data.output_device.wave_format = WAVE_FORMAT_UNKNOWN;    
    WAVEFORMATEX* mix_format = {};
    
    data.output_device.audio_client->GetMixFormat(&mix_format);
    WAVEFORMATEXTENSIBLE* extensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mix_format);

    if(mix_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        // SubFormat == X causes a call to memcmp
        if(mem::compare(mem::to_const_bytes(Slice(&extensible->SubFormat, 1)),
            mem::to_const_bytes(Slice(&KSDATAFORMAT_SUBTYPE_PCM, 1))))
        {
            data.output_device.wave_format = WAVE_FORMAT_PCM;
        }
        else if(mem::compare(mem::to_const_bytes(Slice(&extensible->SubFormat, 1)),
            mem::to_const_bytes(Slice(&KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 1))))
        {
            data.output_device.wave_format = WAVE_FORMAT_IEEE_FLOAT;
        }
        else
        {
            WASAPIFailOn(true, "unknown data format");
        }
    }
    else if(mix_format->wFormatTag == WAVE_FORMAT_PCM)
    {
        data.output_device.wave_format = WAVE_FORMAT_PCM;
    }
    else if(mix_format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        data.output_device.wave_format = WAVE_FORMAT_IEEE_FLOAT;
    }
    else
    {
        WASAPIFailOn(true, "unknown data format");
    }

    WAVEFORMATEX* format = {};
    WAVEFORMATEX* closest_format = nullptr;
    HRESULT result = data.output_device.audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, mix_format, &closest_format);
    if(SUCCEEDED(result) && closest_format != nullptr)
    {
        format = closest_format;
        CoTaskMemFree(mix_format);
    }
    else
    {
        format = mix_format;
    }

    data.output_device.channels = format->nChannels;
    data.output_device.samples_per_sec = format->nSamplesPerSec;
    data.output_device.bits_per_sample = format->wBitsPerSample;

    data.output_device.audio_client->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        0,
        0,
        format,
        nullptr
    );

    CoTaskMemFree(format);

    data.output_device.audio_client->GetBufferSize(&data.output_device.frame_count);

    data.output_device.audio_client->GetService(
        __uuidof(IAudioRenderClient),
        reinterpret_cast<void**>(&data.output_device.render_client)
    );

    data.event_handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    data.output_device.audio_client->SetEventHandle(data.event_handle);

    WASAPIDebugInfo("Output service was installed with:"
        "\n\tFormat: {}"
        "\n\tChannels: {}"
        "\n\tSamplesPerSec: {}"
        "\n\tBitsPerSample: {}",
        data.output_device.wave_format == WAVE_FORMAT_PCM ? StringView("I32") : StringView("Float"),
        data.output_device.channels,
        data.output_device.samples_per_sec,
        data.output_device.bits_per_sample
    );
}

void WASAPIDriver::shutdown()
{
    CloseHandle(data.event_handle);
    data.output_device.render_client->Release();
    data.output_device.audio_client->Release();
    data.output_device.device->Release();
}

u32 WASAPIDriver::output_get_samples_per_sec()
{
    return data.output_device.samples_per_sec;
}

void WASAPIDriver::output_start()
{
    data.output_device.audio_client->Start();
}

void WASAPIDriver::output_stop()
{
    data.output_device.audio_client->Stop();
}

bool WASAPIDriver::output_wait_for_event()
{
    return WaitForSingleObject(data.event_handle, INFINITE) == WAIT_OBJECT_0;
}

u32 WASAPIDriver::output_get_frame_count()
{
    u32 padding = 0;
    data.output_device.audio_client->GetCurrentPadding(&padding);
    u32 frames_available = data.output_device.frame_count - padding;
    return frames_available;
}

void WASAPIDriver::output_send_frames(const Slice<Audio::Frame>& frames)
{
    u32 frame_count = math::min(output_get_frame_count(), frames.len);

    u8* buffer_out = nullptr;
    data.output_device.render_client->GetBuffer(frame_count, &buffer_out);
    
    f32* buffer_out_f = reinterpret_cast<f32*>(buffer_out);

    // TODO: 16/24/32 PCM format support
    if(data.output_device.wave_format == WAVE_FORMAT_IEEE_FLOAT)
    {
        if(data.output_device.channels >= 2)
        {
            for(usize i = 0; i < frame_count; i++)
            {
                Audio::Frame frame = frames[i];

                buffer_out_f[(i * data.output_device.channels) + 0] = f32(frame.left) / 32768.F;
                buffer_out_f[(i * data.output_device.channels) + 1] = f32(frame.right) / 32768.f;
                
                for(usize j = Audio::OutputChannels; j < data.output_device.channels; i++)
                {
                    buffer_out_f[(i * data.output_device.channels) + j] = 0.F;
                }
            }
        }
        else if(data.output_device.channels == 1)
        {
            for(usize i = 0; i < frame_count; i++)
            {
                Audio::Frame frame = frames[i];

                f32 normal = (f32(frame.left + frame.right) / 2.F) / 32768.F;
                if(normal > 1.F)
                {
                    normal = 1.F;
                }
                else if(normal < -1.F)
                {
                    normal = -1.F;
                }
                buffer_out_f[i] = normal;
            }
        }
    }

    data.output_device.render_client->ReleaseBuffer(frame_count, 0);
}


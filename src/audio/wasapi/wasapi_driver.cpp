#include "audio/wasapi/wasapi_driver.h"

#include "math/funcs.h"


InternalAudio::AudioAdapter WASAPIDriver::get_vtable()
{
    return InternalAudio::AudioAdapter
    {
        .initialize = &WASAPIDriver::initialize,
        .shutdown = &WASAPIDriver::shutdown,
        .output_get_format = &WASAPIDriver::output_get_format,
        .output_get_channels = &WASAPIDriver::output_get_channels,
        .output_get_samples_per_sec = &WASAPIDriver::output_get_samples_per_sec,
        .output_get_bits_per_sample = &WASAPIDriver::output_get_bits_per_sample,
        .output_start = &WASAPIDriver::output_start,
        .output_stop = &WASAPIDriver::output_stop,
        .output_wait_for_event = &WASAPIDriver::output_wait_for_event,
        .output_get_buffer = &WASAPIDriver::output_get_buffer,
        .output_release_buffer = &WASAPIDriver::output_release_buffer,
    };
}

void WASAPIDriver::initialize(const mem::Allocator& allocator)
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
    
    data.output_device.format = Audio::Format::Unknown;    
    WAVEFORMATEX* mix_format = {};
    
    data.output_device.audio_client->GetMixFormat(&mix_format);
    WAVEFORMATEXTENSIBLE* extensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mix_format);

    if(mix_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        // SubFormat == X causes a call to memcmp
        if(mem::compare(mem::to_const_bytes(Slice(&extensible->SubFormat, 1)),
            mem::to_const_bytes(Slice(&KSDATAFORMAT_SUBTYPE_PCM, 1))))
        {
            data.output_device.format = Audio::Format::PCM;
        }
        else if(mem::compare(mem::to_const_bytes(Slice(&extensible->SubFormat, 1)),
            mem::to_const_bytes(Slice(&KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 1))))
        {
            data.output_device.format = Audio::Format::IEEEFloat;
        }
        else
        {
            WASAPIFailOn(true, "unknown data format");
        }
    }
    else if(mix_format->wFormatTag == WAVE_FORMAT_PCM)
    {
        data.output_device.format = Audio::Format::PCM;
    }
    else if(mix_format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        data.output_device.format = Audio::Format::IEEEFloat;
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
        data.output_device.format == Audio::Format::PCM ? StringView("I32") : StringView("Float"),
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

Audio::Format WASAPIDriver::output_get_format()
{
    return data.output_device.format;
}

u32 WASAPIDriver::output_get_channels()
{
    return data.output_device.channels;
}

u32 WASAPIDriver::output_get_samples_per_sec()
{
    return data.output_device.samples_per_sec;
}

u32 WASAPIDriver::output_get_bits_per_sample()
{
    return data.output_device.bits_per_sample;
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

Opaque* WASAPIDriver::output_get_buffer(u32* out_frame_count)
{
    u32 padding = 0;
    data.output_device.audio_client->GetCurrentPadding(&padding);
    u32 frames_available = data.output_device.frame_count - padding;

    *out_frame_count = frames_available;
    if (frames_available == 0)
    {
        return nullptr;
    }

    BYTE* buffer = nullptr;
    data.output_device.render_client->GetBuffer(frames_available, &buffer);
    return reinterpret_cast<Opaque*>(buffer);
}

void WASAPIDriver::output_release_buffer(u32 frame_count)
{
    data.output_device.render_client->ReleaseBuffer(frame_count, 0);
}


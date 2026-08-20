#include "Audio/WASAPI/WASAPIDriver.hpp"

#include "math/funcs.h"


alignas(alignof(WASAPIDriver::InternalData)) static u8 place_holder_memory[sizeof(WASAPIDriver::InternalData)]{};
static WASAPIDriver::InternalData& get_data()
{
	return *reinterpret_cast<WASAPIDriver::InternalData*>(place_holder_memory);
}

WASAPIDriver::WASAPIDriver(Mem::Allocator& allocator)
{
    WASAPIDebugInfo("Initializing WASAPI Driver...");

    // Ensures constructors are call.
    Core::Mem::Placement(get_data(), allocator);

    IMMDeviceEnumerator* enumerator = nullptr;

    CoCreateInstance(
        __uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&enumerator)
    );

    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &get_data().output_device.device);
    enumerator->Release();

    get_data().output_device.device->Activate(
        __uuidof(IAudioClient), CLSCTX_ALL,
        nullptr, reinterpret_cast<void**>(&get_data().output_device.audio_client)
    );
    
    get_data().output_device.wave_format = WAVE_FORMAT_UNKNOWN;    
    WAVEFORMATEX* mix_format = {};
    
    get_data().output_device.audio_client->GetMixFormat(&mix_format);
    WAVEFORMATEXTENSIBLE* extensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mix_format);

    if(mix_format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    {
        // SubFormat == X causes a call to memcmp
        if(Mem::compare(Mem::to_const_bytes(Slice(&extensible->SubFormat, 1)),
            Mem::to_const_bytes(Slice(&KSDATAFORMAT_SUBTYPE_PCM, 1))))
        {
            get_data().output_device.wave_format = WAVE_FORMAT_PCM;
        }
        else if(Mem::compare(Mem::to_const_bytes(Slice(&extensible->SubFormat, 1)),
            Mem::to_const_bytes(Slice(&KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, 1))))
        {
            get_data().output_device.wave_format = WAVE_FORMAT_IEEE_FLOAT;
        }
        else
        {
            WASAPIFailOn(true, "unknown data format");
        }
    }
    else if(mix_format->wFormatTag == WAVE_FORMAT_PCM)
    {
        get_data().output_device.wave_format = WAVE_FORMAT_PCM;
    }
    else if(mix_format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
    {
        get_data().output_device.wave_format = WAVE_FORMAT_IEEE_FLOAT;
    }
    else
    {
        WASAPIFailOn(true, "unknown data format");
    }

    WAVEFORMATEX* format = {};
    WAVEFORMATEX* closest_format = nullptr;
    HRESULT result = get_data().output_device.audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, mix_format, &closest_format);
    if(SUCCEEDED(result) && closest_format != nullptr)
    {
        format = closest_format;
        CoTaskMemFree(mix_format);
    }
    else
    {
        format = mix_format;
    }

    get_data().output_device.channels = format->nChannels;
    get_data().output_device.samples_per_sec = format->nSamplesPerSec;
    get_data().output_device.bits_per_sample = format->wBitsPerSample;

    get_data().output_device.audio_client->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        0,
        0,
        format,
        nullptr
    );

    CoTaskMemFree(format);

    get_data().output_device.audio_client->GetBufferSize(&get_data().output_device.frame_count);

    get_data().output_device.audio_client->GetService(
        __uuidof(IAudioRenderClient),
        reinterpret_cast<void**>(&get_data().output_device.render_client)
    );

    get_data().event_handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    get_data().output_device.audio_client->SetEventHandle(get_data().event_handle);

    WASAPIDebugInfo("Output service was installed with:"
        "\n\tFormat: {}"
        "\n\tChannels: {}"
        "\n\tSamplesPerSec: {}"
        "\n\tBitsPerSample: {}",
        get_data().output_device.wave_format == WAVE_FORMAT_PCM ?
        Collections::StringView("I32") : Collections::StringView("Float"),
        get_data().output_device.channels,
        get_data().output_device.samples_per_sec,
        get_data().output_device.bits_per_sample
    );
}

WASAPIDriver::~WASAPIDriver()
{
    CloseHandle(get_data().event_handle);
    get_data().output_device.render_client->Release();
    get_data().output_device.audio_client->Release();
    get_data().output_device.device->Release();

    Core::Mem::Destruct(get_data());
}

u32 WASAPIDriver::output_get_samples_per_sec()
{
    return get_data().output_device.samples_per_sec;
}

void WASAPIDriver::output_start()
{
    get_data().output_device.audio_client->Start();
}

void WASAPIDriver::output_stop()
{
    get_data().output_device.audio_client->Stop();
}

bool WASAPIDriver::output_wait_for_event()
{
    return WaitForSingleObject(get_data().event_handle, INFINITE) == WAIT_OBJECT_0;
}

u32 WASAPIDriver::output_get_frame_count()
{
    u32 padding = 0;
    get_data().output_device.audio_client->GetCurrentPadding(&padding);
    u32 frames_available = get_data().output_device.frame_count - padding;
    return frames_available;
}

void WASAPIDriver::output_send_frames(const Slice<Audio::Frame>& frames)
{
    // lattency could make it require more frames that the ones being send
    u32 frame_count = Math::min(output_get_frame_count(), frames.len);

    u8* buffer_out = nullptr;
    get_data().output_device.render_client->GetBuffer(frame_count, &buffer_out);
    
    f32* buffer_out_f = reinterpret_cast<f32*>(buffer_out);

    // TODO: 16/24/32 PCM format support
    if(get_data().output_device.wave_format == WAVE_FORMAT_IEEE_FLOAT)
    {
        if(get_data().output_device.channels >= 2)
        {
            for(usize i = 0; i < frame_count; i++)
            {
                Audio::Frame frame = frames[i];

                buffer_out_f[(i * get_data().output_device.channels) + 0] = frame.left;
                buffer_out_f[(i * get_data().output_device.channels) + 1] = frame.right;
                
                for(usize j = Audio::OutputChannels; j < get_data().output_device.channels; i++)
                {
                    buffer_out_f[(i * get_data().output_device.channels) + j] = 0.F;
                }
            }
        }
        else if(get_data().output_device.channels == 1)
        {
            for(usize i = 0; i < frame_count; i++)
            {
                Audio::Frame frame = frames[i];

                f32 normal = Math::clamp(
                    f32(frame.left + frame.right) / 2.F,
                    -1.F, 1.F
                );

                buffer_out_f[i] = normal;
            }
        }
    }

    get_data().output_device.render_client->ReleaseBuffer(frame_count, 0);
}


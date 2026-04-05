#include "audio/wasapi/wasapi_driver.h"

#include "math/funcs.h"


Audio::VTable WASAPIDriver::get_vtable()
{
    return Audio::VTable
    {
        .initialize = &WASAPIDriver::initialize,
        .shutdown = &WASAPIDriver::shutdown,
    };
}

void WASAPIDriver::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    IMMDeviceEnumerator* enumerator = nullptr;

    CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                    __uuidof(IMMDeviceEnumerator), (void**)&enumerator);

    enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &data.output_device.device);
    enumerator->Release();

    data.output_device.device->Activate(__uuidof(IAudioClient), CLSCTX_ALL,
                    nullptr, (void**)&data.output_device.audio_client);

    WAVEFORMATEX format = {};
    format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    format.nChannels = 2;
    format.nSamplesPerSec = 48000;
    format.wBitsPerSample = 32;
    format.nBlockAlign = format.nChannels * (format.wBitsPerSample / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

    REFERENCE_TIME bufferDuration = 10000000; // 1 second (in 100ns units)

    data.output_device.audio_client->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        bufferDuration,
        0,
        &format,
        nullptr
    );

    UINT32 bufferFrameCount = 0;
    data.output_device.audio_client->GetBufferSize(&bufferFrameCount);

    data.output_device.audio_client->GetService(__uuidof(IAudioRenderClient),
                        (void**)&data.output_device.render_client);

    data.output_device.audio_client->Start();
}

void WASAPIDriver::shutdown()
{
    data.output_device.render_client->Release();
    data.output_device.audio_client->Release();
    data.output_device.device->Release();
}

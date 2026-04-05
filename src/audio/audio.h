#pragma once
#include "audio/audio_structs.h"
#include "mem/allocator.h"


struct Audio
{
    enum class DriverType
    {
        Unknown = 0,

        Wasapi,
        AAudio,

#if defined(BREAD_WIN32)
        Default = Wasapi,
#elif defined(BREAD_ANDROID)
        Default = AAudio,
#endif
    };

    struct VTable
    {
        VTFunc(void, initialize, const mem::Allocator&);
        VTFunc(void, shutdown);
    };

    static inline VTable vtable;

    static void initialize(const mem::Allocator& allocator, DriverType driver);
    VTFuncDefS(shutdown);
};


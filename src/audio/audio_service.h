#pragma once
#include "mem/allocator.h"
#include "systems/system.h"


struct AudioServiceCreateInfo
{
    mem::Allocator allocator;
};

struct AudioService : System<AudioService>
{
    static constexpr SystemDependency Dependencies[] =
    {
        SystemDependency::of("IdentitySystem")
    };

    static constexpr StringView _name = "AudioService";
    static constexpr SystemInfo get_system_info()
    {
        return System::get_system_info_with_name(_name);
    }

    mem::Allocator allocator;

    void initialize(const SystemInitializeInfo& info);
    void shutdown();
};

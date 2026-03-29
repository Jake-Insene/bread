#pragma once
#include "core/header.h"
#include "collections/string_view.h"
#include "collections/event.h"
#include "input/input.h"
#include "mem/allocator.h"



struct SystemInitializeInfo
{
    mem::Allocator allocator;
};

struct SystemRuntime
{
    Event<void(*)(Opaque*, const SystemInitializeInfo& init_info)> initialize;
    Event<void(*)(Opaque*)> shutdown;
    Event<void(*)(Opaque*, const InputEvent&)> on_event;
};

struct SystemDependency
{
    StringView name;

    static constexpr SystemDependency of(StringView system_name)
    {
        return SystemDependency{ .name = system_name };
    }
};

enum class SystemFlags
{
    Tick = Bit(0),
    OnEvent = Bit(1),
};

EnableBitOp(SystemFlags);

struct SystemInfo
{
    SystemRuntime runtime;
    Slice<const SystemDependency> dependencies;
    SystemFlags flags;
    usize size_in_bytes;
    StringView name;
};

template<typename T>
struct System
{
    static constexpr SystemDependency Dependencies[] = { SystemDependency() };

    static constexpr SystemRuntime _get_system_runtime()
    {
        SystemRuntime runtime = {};
        runtime.initialize.bind([](Opaque* system_ref, const SystemInitializeInfo& info)
        {
            T* system = system_ref->cast<T*>();
            ConstructObject(system);
            system->initialize(info);
        });

        runtime.shutdown.bind([](Opaque* system_ref)
        {
            T* system = system_ref->cast<T*>();
            system->shutdown();
            DestructObject(system);
        });

        runtime.on_event.bind([](Opaque* system_ref, const InputEvent& e)
        {
            T* system = system_ref->cast<T*>();
            system->on_event(e);
        });

        return runtime;
    }

    static constexpr SystemInfo get_system_info_with_name(StringView name)
    {
        return SystemInfo
        {
            .runtime = _get_system_runtime(),
            .dependencies = T::Dependencies,
            .flags = SystemFlags(),
            .size_in_bytes = sizeof(T),
            .name = name,
        };
    }

    static constexpr SystemInfo get_system_info_with_name_and_flags(StringView name, SystemFlags flags)
    {
        return SystemInfo
        {
            .runtime = _get_system_runtime(),
            .dependencies = T::Dependencies,
            .flags = flags,
            .size_in_bytes = sizeof(T),
            .name = name,
        };
    }
};


struct IdentitySystem : System<IdentitySystem>
{
    void initialize(const SystemInitializeInfo&) {}
    void shutdown() {}

    void on_event(const InputEvent&) {}

    static constexpr StringView _name = "IdentitySystem";
    static constexpr SystemInfo get_system_info()
    {
        return System::get_system_info_with_name(_name);
    }
};


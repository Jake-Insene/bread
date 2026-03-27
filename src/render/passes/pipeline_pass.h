#pragma once
#include "collections/event.h"
#include "collections/string_view.h"
#include "mem/allocator.h"



struct RenderCommandEncoder;

struct PipelinePassInitializeInfo
{
    mem::Allocator allocator;
};

struct PipelinePassRuntime
{
    Event<void(*)(Opaque*, const PipelinePassInitializeInfo& init_info)> initialize;
    Event<void(*)(Opaque*)> shutdown;
    Event<void(*)(Opaque*, RenderCommandEncoder&)> encode;
};

struct PipelinePassDependency
{
    StringView name;

    static constexpr PipelinePassDependency of(StringView pass_name)
    {
        return PipelinePassDependency{ .name = pass_name };
    }
};

struct PipelinePassInfo
{
    PipelinePassRuntime runtime;
    Slice<const PipelinePassDependency> dependencies;
    usize size_in_bytes;
    StringView name;
};

template<typename T>
struct PipelinePass
{
    static constexpr PipelinePassDependency Dependencies[] = { PassDependency() };

    static constexpr PipelinePassRuntime _get_pass_runtime()
    {
        PipelinePassRuntime runtime = {};
        runtime.initialize.bind([](Opaque* system_ref, const PipelinePassInitializeInfo& info)
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

        runtime.encode.bind([](Opaque* system_ref, RenderCommandEncoder& encoder)
        {
            T* system = system_ref->cast<T*>();
            system->encode(encoder);
        });

        return runtime;
    }

    static constexpr PipelinePassInfo get_pass_info_with_name(StringView name)
    {
        return PipelinePassInfo
        {
            .runtime = _get_pass_runtime(),
            .dependencies = T::Dependencies,
            .size_in_bytes = sizeof(T),
            .name = name,
        };
    }
};



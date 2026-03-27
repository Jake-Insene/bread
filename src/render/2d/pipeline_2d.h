#pragma once
#include "systems/system.h"


struct Pipeline2D : System<Pipeline2D>
{
    static constexpr SystemDependency Dependencies[] =
    {
        SystemDependency::of("RenderDevice"),
        SystemDependency::of("ResourceManager"),
    };

    static constexpr StringView _name = "SceneRenderer2D";
    static constexpr SystemInfo get_system_info()
    {
        return System::get_system_info_with_name(_name);
    }

    void initialize(const SystemInitializeInfo& info);
    void shutdown();

    void on_event(const InputEvent&) {}
};

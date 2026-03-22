#pragma once
#include "collections/array.h"
#include "systems/system.h"


struct SystemManager
{
    mem::Allocator allocator;

    enum class SystemState
    {
        Unknown = 0,
        Valid,
        Invalid,
    };

    struct SystemInstance
    {
        SystemInfo info;
        SystemState state;
        Opaque* instance;
    };
    Array<SystemInstance> systems;
    Array<usize> flow_order;

    void initialize(const mem::Allocator& _allocator);
    void shutdown();

    template<typename T>
    requires IsBaseOf<System<T>, T>
    T* get_system()
    {
        return _get_system(T::get_system_info())->template cast<T*>();
    }

    void allocate_systems(const Slice<SystemInfo>& requested_systems);
    void deallocate_systems();

    void _create_flow_order();

    void _initialize_systems_instance();
    void _shutdown_systems_instance();

    SystemInstance* _get_instance_by_name(StringView name);
    usize _get_instance_index_by_name(StringView name);

    Opaque* _get_system(const SystemInfo& info);
};

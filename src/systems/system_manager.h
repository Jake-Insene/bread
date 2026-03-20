#pragma once
#include "collections/array.h"
#include "systems/system.h"


struct SystemManager
{
    mem::Allocator allocator;

    struct SystemInstance
    {
        SystemInfo info;
        Opaque* instance;
    };
    Array<SystemInstance> systems;

    void init(const mem::Allocator& _allocator);
    void destroy();

    template<typename T>
    requires IsBaseOf<System<T>, T>
    T* get_system()
    {
        return _get_system(T::get_system_info())->template cast<T*>();
    }

    void allocate_systems(const Slice<SystemInfo>& requested_systems);
    void deallocate_systems();

    void _initialize_systems_instance();
    void _shutdown_systems_instance();

    Opaque* _get_system(const SystemInfo& info);
};

#include "systems/system_manager.h"

#include "collections/reverse_iterator.h"


void SystemManager::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    systems = Array<SystemInstance>::with_allocator(allocator);
    flow_order = Array<usize>::with_size(allocator, 4);
}

void SystemManager::shutdown()
{
    flow_order.destroy();
    systems.destroy();
}

void SystemManager::allocate_systems(const Slice<SystemInfo>& requested_systems)
{
    for(const SystemInfo& system_info : requested_systems)
    {
        Slice<u8> instance_bytes = allocator.alloc(system_info.size_in_bytes, 16);

        SystemInstance instance =
        {
            .info = system_info,
            .state = SystemState::Unknown,
            .instance = reinterpret_cast<Opaque*>(instance_bytes.ptr()),
        };

        (void)systems.add(instance);
    }

    _create_flow_order();

    _initialize_systems_instance();
}

void SystemManager::deallocate_systems()
{
    _shutdown_systems_instance();
    
    for(SystemInstance& instance : systems.iter())
    {
        allocator.free(mem::to_bytes(Slice(instance.instance, 1)));
    }

    systems.clear();
}

void SystemManager::tick()
{
    for(SystemInstance& instance : systems.iter())
    {
        SystemFlags flags = instance.info.flags;

        if(HasValue(flags & SystemFlags::Tick))
        {
            // TODO: Add tick
        }
    }
}

void SystemManager::tick_event(const InputEvent& event)
{
    for(SystemInstance& instance : systems.iter())
    {
        SystemFlags flags = instance.info.flags;

        if(HasValue(flags & SystemFlags::OnEvent))
        {
            instance.info.runtime.on_event.call(instance.instance, event);
        }
    }
}

void SystemManager::_create_flow_order()
{
    flow_order.clear();

    usize i = 0;
    while(flow_order.count != systems.count)
    {
        ArrayIterator<usize> iter = flow_order.iter();
        if(iter.find(i) != iter.end())
        {
            i++;
            continue;
        }

        SystemInstance& instance = systems.get(i);
        bool can_be_inserted = true;

        for(usize system_index = 0; system_index < systems.count; system_index++)
        {
            if(i == system_index) continue;
            if(can_be_inserted == false) break;

            for(usize dependecy_index = 0; dependecy_index < instance.info.dependencies.len; dependecy_index++)
            {
                const SystemDependency& dependency = instance.info.dependencies[dependecy_index];
                usize required_system_index = _get_instance_index_by_name(dependency.name);
                if(required_system_index == MaxValue<usize>) continue;

                if(iter.find(required_system_index) == iter.end())
                {
                    can_be_inserted = false;
                }
            }
        }

        if(can_be_inserted)
        {
            (void)flow_order.add(i);
        }

        i++;
        if(i == systems.count) { i = 0; }
    }
}

void SystemManager::_initialize_systems_instance()
{
    for(usize system_index : flow_order.iter())
    {
        SystemInstance& instance = systems.get(system_index);

        SystemInitializeInfo info =
        {
            .allocator = allocator,
        };
        instance.info.runtime.initialize.call(instance.instance, info);

        instance.state = SystemState::Valid;
    }
}

void SystemManager::_shutdown_systems_instance()
{
    ReverseIterator<usize> iterator = ReverseIterator<usize>::from_iter(flow_order.iter());
    for(usize i : iterator)
    {
        SystemInstance& instance = systems.get(i);
        instance.info.runtime.shutdown.call(instance.instance);
        instance.state = SystemState::Invalid;
    }
}

SystemManager::SystemInstance* SystemManager::_get_instance_by_name(StringView name)
{
    static SystemInstance identity = {};
    if(name.equals("IdentitySystem"))
    {
        return &identity;
    }

    for(SystemInstance& instance : systems.iter())
    {
        if(instance.info.name.equals(name))
        {
            return &instance;
        }
    }

    return nullptr;
}

usize SystemManager::_get_instance_index_by_name(StringView name)
{
    if(name.equals("IdentitySystem"))
    {
        return MaxValue<usize>;
    }

    for(usize i = 0; i < systems.count; i++)
    {
        SystemInstance& instance = systems.get(i);
        if(instance.info.name.equals(name))
        {
            return i;
        }
    }

    return MaxValue<usize>;
}

Opaque* SystemManager::_get_system(const SystemInfo& info)
{
    if(SystemInstance* instance = _get_instance_by_name(info.name))
    {
        return instance->instance;
    }

    return nullptr;
}

#include "objects/object.h"

#include "objects/object_allocator.h"
#include "objects/scene_manager.h"


void Object::_bind_vtable(VTable&)
{}

void Object::handle_internal_update(f64 dt)
{
    for(auto child : data.childs)
    {
        child->handle_internal_update(dt);
    }
    
    if(has_internal_update())
    {
        ObjectCall(internal_update, dt);
    }
}

void Object::handle_update(f64 dt)
{
    for(auto child : data.childs)
    {
        child->handle_update(dt);
    }
    
    if(can_update())
    {
        ObjectCall(update, dt);
    }
}

void Object::handle_render()
{
    for(auto child : data.childs)
    {
        child->handle_render();
    }
    
    if(can_render())
    {
        ObjectCall(render);
    }
}

void Object::add_child(Object *obj)
{
    Object* child = data.childs.add(obj);
    
    child->data.parent = this;
    
    if(is_in_scene())
    {
        ObjectCallRef(child, start);
    }
}

void Object::init(const CreateInfo& info)
{
    allocator = info.allocator;
    data.name = String::with_allocator(allocator);
    data.parent = nullptr;
    data.childs = Array<Object*>::with_allocator(allocator);
    
    data.flags.clear();
}

void Object::deinit()
{
    data.name.destroy();
    
    for(Object* child : data.childs)
    {
        DestroyObject(child);
    }

    data.childs.destroy();
}

void Object::start()
{
    // This function is only called in SceneManager when you change the scene
    // and add_child only when the parent is already into the scene.
    data.flags.set(FLAG_IN_SCENE, true);

    for(auto& child : data.childs)
    {
        ObjectCallRef(child, start);
    }
}

void Object::exit()
{
    data.flags.clear();
    for(auto child : data.childs)
    {
        ObjectCallRef(child, exit);
    }
}

void Object::event(const InputEvent& event)
{
    for(auto child : data.childs)
    {
        if(child->can_handle_event())
        {
            ObjectCallRef(child, event, event);
        }
    }
}

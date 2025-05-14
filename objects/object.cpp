#include "objects/object.h"

#include "objects/object_allocator.h"
#include "objects/scene_manager.h"


void Object::_bind_vtable(VTable&)
{}

void Object::handle_internal_update(f64 dt)
{
    for(auto it : data.childs)
    {
        it.value->handle_internal_update(dt);
    }
    
    if(has_internal_update())
    {
        ObjectCall(internal_update, dt);
    }
}

void Object::handle_update(f64 dt)
{
    for(auto it : data.childs)
    {
        it.value->handle_update(dt);
    }
    
    if(can_update())
    {
        ObjectCall(update, dt);
    }
}

void Object::handle_render()
{
    for(auto it : data.childs)
    {
        it.value->handle_render();
    }
    
    if(can_render())
    {
        ObjectCall(render);
    }
}

void Object::add_child(Object *obj)
{
    Object* child = data.childs.insert(obj->id, obj);
    child->data.parent = this;
    
    if(is_in_scene())
    {
        ObjectCallRef(child, start);
    }
}

void Object::remove_child(Object* child)
{
    data.childs.remove(child->id);
    ObjectCallRef(child, exit);
    DestroyObject(child);
}

void Object::queue_free()
{
    SceneManager::_queue_free(get_parent(), this);
}

void Object::init(const CreateInfo& info)
{
    allocator = info.allocator;
    data.name = String::with_allocator(allocator);
    data.parent = nullptr;
    data.childs = HashMap<ObjectID, Object*>::with_allocator(allocator);
    
    data.flags.clear();
}

void Object::deinit()
{
    data.name.destroy();
    
    for(auto& it : data.childs)
    {
        DestroyObject(it.value);
    }

    data.childs.destroy();
}

void Object::start()
{
    // This function is only called in SceneManager when you change the scene
    // and add_child only when the parent is already into the scene.
    data.flags.set(FLAG_IN_SCENE, true);

    for(auto it: data.childs)
    {
        ObjectCallRef(it.value, start);
    }
}

void Object::exit()
{
    data.flags.clear();
    for(auto it : data.childs)
    {
        ObjectCallRef(it.value, exit);
    }
}

void Object::event(const InputEvent& event)
{
    for(auto it : data.childs)
    {
        if(it.value->can_handle_event())
        {
            ObjectCallRef(it.value, event, event);
        }
    }
}

#include "object/object.h"

#include "input/input.h"
#include "object/object_allocator.h"
#include "scene/scene_manager.h"


void Object::_bind_vtable(VTable&)
{}

void Object::handle_internal_update(f64 dt)
{
    for(auto it : data.childs)
    {
        it.second->handle_internal_update(dt);
    }
    
    if(has_mark(MARK_INTERNAL_UPDATE))
    {
        ObjectCall(internal_update, dt);
    }
}

void Object::handle_update(f64 dt)
{
    for(auto it : data.childs)
    {
        it.second->handle_update(dt);
    }
    
    if(has_mark(MARK_UPDATE))
    {
        ObjectCall(update, dt);
    }
}

void Object::handle_render()
{
    for(auto it : data.childs)
    {
        it.second->handle_render();
    }
    
    if(has_mark(MARK_RENDER))
    {
        ObjectCall(render);
    }
}

void Object::set_group(u64 group_bit, bool value)
{
    data.bit_groups.set(group_bit, value);
}

void Object::add_child(Object *obj)
{
    Object* child = data.childs.insert(obj->id, obj);
    child->data.parent = this;
    
    if(has_mark(MARK_IN_SCENE))
    {
        ObjectCallRef(child, enter);
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
    
    data.marks.clear();
}

void Object::deinit()
{
    data.name.destroy();
    
    for(auto& it : data.childs)
    {
        DestroyObject(it.second);
    }

    data.childs.destroy();
}

void Object::enter()
{
    // This function is only called in SceneManager when you change the scene
    // and add_child only when the parent is already into the scene.
    mark(MARK_IN_SCENE);

    for(auto& it: data.childs)
    {
        ObjectCallRef(it.second, enter);
    }
}

void Object::exit()
{
    data.marks.clear();
    for(auto it : data.childs)
    {
        ObjectCallRef(it.second, exit);
    }
}

void Object::event(const InputEvent&)
{}

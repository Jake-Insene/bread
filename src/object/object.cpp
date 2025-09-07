#include "object/object.h"

#include "input/input.h"
#include "object/object_allocator.h"
#include "scene/scene_manager.h"


void Object::_bind_vtable(VTable&)
{}

void Object::set_viewport(Viewport* new_vp)
{
    if (data.viewport == new_vp)
        return;

    for (usize i = 0; i < data.childs.count; i++)
    {
        data.childs[i]->set_viewport(new_vp);
    }

    data.viewport = new_vp;
}

void Object::handle_internal_update(f32 dt)
{
    for (usize i = 0; i < data.childs.count; i++)
    {
        data.childs[i]->handle_internal_update(dt);
    }
    
    if(has_mark(MARK_INTERNAL_UPDATE))
    {
        ObjectCall(internal_update, dt);
    }
}

void Object::handle_update(f32 dt)
{
    for (usize i = 0; i < data.childs.count; i++)
    {
        data.childs[i]->handle_update(dt);
    }
    
    if(has_mark(MARK_UPDATE))
    {
        ObjectCall(update, dt);
    }
}

void Object::handle_render()
{
    for (usize i = 0; i < data.childs.count; i++)
    {
        data.childs[i]->handle_render();
    }
    
    if(has_mark(MARK_RENDER))
    {
        ObjectCall(render);
    }
}

void Object::handle_event(const InputEvent& e)
{
    for (usize i = 0; i < data.childs.count; i++)
    {
        data.childs[i]->handle_event(e);
    }

    if (has_mark(MARK_EVENT))
    {
        ObjectCall(event, e);
    }
}

void Object::set_group(u64 group_bit, bool value)
{
    if (value)
        data.bit_groups.set(group_bit);
    else
        data.bit_groups.unset(group_bit);
}

void Object::add_child(Object *obj)
{
    Object* child = data.childs.add(obj);
    child->data.parent = this;
    child->set_viewport(get_viewport());

    if (has_mark(MARK_IN_SCENE))
    {
        ObjectCallRef(child, enter);
    }
}

void Object::remove_child(Object* child)
{
    data.childs.remove_equal(child);
    ObjectCallRef(child, exit);
    DestroyObject(child);
}

void Object::queue_free()
{
    if (has_mark(MARK_QUEUE_FREE))
        return;

    mark(MARK_QUEUE_FREE);
    SceneManager::_queue_free(get_parent(), this);
}

void Object::init(const CreateInfo& info)
{
    allocator = info.allocator;
    data.name = String::with_allocator(allocator);
    data.parent = nullptr;
    data.childs = Array<Object*>::with_allocator(allocator);
    
    data.marks.clear();
}

void Object::deinit()
{
    data.name.destroy();
    
    for(auto& child : data.childs)
    {
        DestroyObject(child);
    }

    data.childs.destroy();
}

void Object::enter()
{
    // This function is only called in SceneManager when you use change_scene(),
    // and add_child() when the parent is already into the scene.
    mark(MARK_IN_SCENE);

    for(auto& child : data.childs)
    {
        child->set_viewport(get_viewport());
        ObjectCallRef(child, enter);
    }
}

void Object::exit()
{
    data.marks.clear();
    for(auto child : data.childs)
    {
        ObjectCallRef(child, exit);
    }
}

void Object::event(const InputEvent&)
{}

#include "object/object.h"

#include "input/input.h"
#include "object/object_allocator.h"
#include "scene/scene_manager.h"


void Object::_bind_vtable(VTable&)
{}

Object* Object::_get_by_id(ObjectID id)
{
    return ObjectAllocator::get_by_id(id);
}

Object* Object::create_from_class(const Class* object_klass)
{
    return ObjectAllocator::allocate_class(object_klass);
}

void Object::set_viewport(Viewport* new_vp)
{
    if (data.viewport == new_vp)
        return;

    for (usize i = 0; i < data.childs.count; i++)
    {
        data.childs.get(i)->set_viewport(new_vp);
    }

    data.viewport = new_vp;
}

void Object::handle_event(const InputEvent& event)
{
    for (usize i = 0; i < data.childs.count; i++)
    {
        data.childs.get(i)->handle_event(event);
    }

    if (has_mark(MARK_EVENT))
    {
        ObjectCall(event, event);
    }
}

void Object::mark(MarkName mark_name)
{
    switch (mark_name)
    {
    case MARK_INTERNAL_UPDATE:
    case MARK_UPDATE:
    case MARK_RENDER:
        SceneManager::_update_object_mark(mark_name, this, true);
        break;
    case MARK_DEALLOCATED:
        SceneManager::_update_object_mark(mark_name, this, true);
        break;
    default:
        break;
    }

    data.marks.set(mark_name);
}

void Object::unmark(MarkName mark_name)
{
    switch (mark_name)
    {
    case MARK_INTERNAL_UPDATE:
    case MARK_UPDATE:
    case MARK_RENDER:
        SceneManager::_update_object_mark(mark_name, this, false);
        break;
    default:
        break;
    }

    data.marks.unset(mark_name);
}

void Object::set_group(GroupName group_name, bool value)
{
    if (value)
        data.bit_groups.set(group_name);
    else
        data.bit_groups.unset(group_name);
}

bool Object::has_group(GroupName group_name) const
{
    return data.bit_groups.is_set(group_name);
}

void Object::add_child(Object* request_child)
{
    Object* child = data.childs.add(request_child);
    child->data.parent = this;
    child->set_viewport(get_viewport());

    if (has_mark(MARK_IN_SCENE))
    {
        ObjectCallRef(child, enter);
    }
}

void Object::remove_child(Object* child)
{
    data.childs.remove(child);
    if (has_mark(MARK_IN_SCENE))
    {
        ObjectCallRef(child, exit);
    }

    ObjectAllocator::destroy_object(child);
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
    
    for(Object* child : data.childs.iter())
    {
        ObjectAllocator::destroy_object(child);
    }

    data.childs.destroy();
}

void Object::enter()
{
    // This function is only called in SceneManager when you use change_scene(),
    // and add_child() when the parent is already into the scene.
    mark(MARK_IN_SCENE);

    for(Object* child : data.childs.iter())
    {
        child->set_viewport(get_viewport());
        ObjectCallRef(child, enter);
    }
}

void Object::exit()
{
    data.marks.clear();
    for(Object* child : data.childs.iter())
    {
        ObjectCallRef(child, exit);
    }
}

void Object::event(const InputEvent&)
{}

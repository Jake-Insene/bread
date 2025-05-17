#include "gui/control.h"

#include "scene/scene_manager.h"


void Control::_bind_vtable(VTable&)
{}

void Control::init(const CreateInfo&)
{
    Object::data.marks.set(MARK_CONTROL, 1);
}

void Control::enter()
{
    Object* parent = get_parent();
    if(parent && !parent->has_mark(MARK_CONTROL))
    {
        SceneManager::_add_root_control(this);
    }
    
}


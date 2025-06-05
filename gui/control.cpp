#include "gui/control.h"

#include "scene/scene_manager.h"


void Control::_bind_vtable(VTable&)
{}

void Control::init(const CreateInfo&)
{
    mark(MARK_CONTROL);
}

void Control::enter()
{
    Object* parent = get_parent();
    if(parent && !parent->has_mark(MARK_CONTROL))
    {
        SceneManager::_add_root_control(this);
    }
    
}


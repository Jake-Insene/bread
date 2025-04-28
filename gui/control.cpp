#include "gui/control.h"

#include "objects/scene_manager.h"


void Control::_bind_vtable(VTable&)
{}

void Control::init(const CreateInfo&)
{
    Object::data.flags.set(FLAG_CONTROL, 1);
}

void Control::start()
{
    Object* parent = get_parent();
    if(parent && !parent->has_flag(FLAG_CONTROL))
    {
        SceneManager::add_root_control(this);
    }
    
}


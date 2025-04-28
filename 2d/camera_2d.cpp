#include "2d/camera_2d.h"

#include "objects/scene_manager.h"

void Camera2D::init(const Object::CreateInfo&)
{}

void Camera2D::start()
{}

void Camera2D::set_enable(bool _enable)
{
    if(enable != _enable)
    {
        enable = _enable;
        if(enable)
        {
            SceneManager::set_camera_2d(this);
        }
    }
}

void Camera2D::set_position(const Vector2&)
{
}

#include "2d/camera_2d.h"

#include "scene/scene_manager.h"

void Camera2D::init(const Object::CreateInfo&)
{}

void Camera2D::enter()
{}

void Camera2D::exit()
{
    if (SceneManager::get_camera_2d() == this)
    {
        SceneManager::set_camera_2d(nullptr);
    }
}

void Camera2D::set_enable(bool _enable)
{
    if (enable != _enable && has_mark(MARK_IN_SCENE))
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

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
    if (data.enable != _enable && has_mark(MARK_IN_SCENE))
    {
        data.enable = _enable;
        if(data.enable)
        {
            SceneManager::set_camera_2d(this);
        }
    }
}

Transform2D Camera2D::get_camera_transform()
{
    Transform2D camera_transform = get_global_transform();
    
    f64 dt = SceneManager::get_delta_time();
    data.old_pos = Vector2::lerp(data.old_pos, camera_transform.get_position(), speed * dt);

    Transform2D transform;
    transform.translate(data.old_pos);
    return transform;
}

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
    const Transform2D camera_transform = get_global_transform();
    const Vector2 camera_position = camera_transform.get_position();
    const Vector2 camera_scale = camera_transform.get_scale();
    const f64 dt = SceneManager::get_delta_time();

    if (data.position_mode == PositionMode::POSITION_CENTERED)
    {
        const Vector2 display_size = Vector2(SceneManager::get_display_target().get_size());
        Vector2 centered_pos = camera_position * camera_scale;
        centered_pos -= (Vector2(display_size.x, -display_size.y) * 0.5);
        
        data.old_pos.x = math::lerp(data.old_pos.x, centered_pos.x, camera_speed.x * dt);
        data.old_pos.y = math::lerp(data.old_pos.y, centered_pos.y, camera_speed.y * dt);
    }
    else
    {
        data.old_pos.x = math::lerp(data.old_pos.x, camera_position.x, camera_speed.x * dt);
        data.old_pos.y = math::lerp(data.old_pos.y, camera_position.y, camera_speed.y * dt);
    }

    Transform2D transform;
    transform.set_scale(camera_scale);
    transform.translate(data.old_pos);
    return transform;
}

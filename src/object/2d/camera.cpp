#include "object/2d/camera.h"


void Camera::init(const CreateInfo& info)
{
    Object::init(info);

    data.old_pos = Vector2();
    data.old_rot = 0.F;
    data.position_mode = POSITION_TOP_LEFT;
    data.transform = Transform2D();

    camera_speed = Vector2(1, 1);
}

void Camera::deinit()
{}

Transform2D Camera::get_camera_transform(const Vector2& viewport_size, f32 dt, bool update)
{
    const Transform2D camera_transform = data.transform;
    const Vector2 camera_scale = camera_transform.get_scale();
    const Vector2 camera_position = camera_transform.get_position() * camera_scale;
    const f32 camera_rot = camera_transform.get_rotation();

    switch (get_position_mode())
    {
    case POSITION_TOP_LEFT:
    {
        if (!update)
        {
            break;
        }

        if (smooth_position)
        {
            data.old_pos.x = math::lerp(data.old_pos.x, camera_position.x, camera_speed.x * dt);
            data.old_pos.y = math::lerp(data.old_pos.y, camera_position.y, camera_speed.y * dt);
        }
        else
        {
            data.old_pos = camera_position;
        }
    }
        break;
    case POSITION_CENTERED:
    {
        if (!update)
        {
            break;
        }

        const Vector2 display_size = viewport_size;
        Vector2 centered_pos = camera_position;
        centered_pos -= (Vector2(display_size.x, -display_size.y) * 0.5);

        if (smooth_position)
        {
            data.old_pos.x = math::lerp(data.old_pos.x, centered_pos.x, camera_speed.x * dt);
            data.old_pos.y = math::lerp(data.old_pos.y, centered_pos.y, camera_speed.y * dt);
        }
        else
        {
            data.old_pos = centered_pos;
        }
    }
        break;
            
    }

    Transform2D transform;
    transform.set_rotation(camera_rot);
    transform.set_scale(camera_scale);
    transform.translate(data.old_pos);
    return transform;
}

void Camera::set_transform(const Transform2D& new_transform)
{
    data.transform = new_transform;
}

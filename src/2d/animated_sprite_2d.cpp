#include "2d/animated_sprite_2d.h"

#include "graphics/render_manager.h"


void AnimatedSprite2D::init(const CreateInfo&)
{
	mark(MARK_RENDER);
	data.current_animation = String::with_allocator(allocator);
}

void AnimatedSprite2D::deinit()
{
	data.current_animation.destroy();
}

void AnimatedSprite2D::internal_update(f32 dt)
{
	data.remain -= f32(dt);

	if (data.remain > 0)
		return;

	data.frame += 1;
	if (data.frame >= animation->get_frame_count(data.current_animation.view()))
	{
		data.frame = 0;

		SpriteAnimation::Animation& anim = animation->get_animation(data.current_animation.view());
		if (anim.loop == false)
		{
			data.playing = false;
			unmark(MARK_INTERNAL_UPDATE);
		}
	}

	SpriteAnimation::SpriteFrame& frame = animation->get_frame(data.current_animation.view(), data.frame);
	data.remain = frame.duration;
}

void AnimatedSprite2D::render()
{
	if (animation == nullptr)
		return;

	if (data.playing == false)
		return;

	SpriteAnimation::SpriteFrame& current_frame = animation->get_frame(
		data.current_animation.view(), data.frame
	);
	Vector2 extent = Vector2(current_frame.sprite->get_size());
	Rect2D rect{ Vector2(), extent };
	Rect2D src_rect{ Vector2(), extent };

	u32 flags = 0;
	if (flip_h)
	{
		flags |= RenderManager::RENDER_FLAG_FLIP_H;
	}
	if (flip_v)
	{
		flags |= RenderManager::RENDER_FLAG_FLIP_V;
	}

	if (centered)
	{
		rect.position = Vector2(rect.size.x / -2.f, rect.size.y / 2.f);
	}

	draw_sprite(
		get_global_transform(), current_frame.sprite->texture_id,
		rect, src_rect, Color(255, 255, 255, 255), flags
	);
}

void AnimatedSprite2D::play(StringView animation_name)
{
	if (!animation || !animation->has_animation(animation_name))
		return;

	if (data.current_animation.equals(animation_name))
	{
		if (data.playing)
			return;
	}

	data.playing = true;
	data.frame = 0;

	data.current_animation.set(animation_name);
	data.remain = animation->get_frame(animation_name, 0).duration;
	mark(MARK_INTERNAL_UPDATE);
}

void AnimatedSprite2D::stop()
{
	data.playing = false;
	data.frame = 0;
	data.current_animation.set("");
	unmark(MARK_INTERNAL_UPDATE);
}

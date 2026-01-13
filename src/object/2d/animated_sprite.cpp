#include "object/2d/animated_sprite.h"

#include "resource/sprite_animation.h"



void AnimatedSprite::init(const CreateInfo& info)
{
	Renderable::init(info);
	data.current_animation = String::with_allocator(allocator);
}

void AnimatedSprite::deinit()
{
	data.current_animation.destroy();

	Renderable::deinit();
}

void AnimatedSprite::update(f32 dt)
{
	if(data.playing == false)
		return;

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
		}
	}

	SpriteAnimation::SpriteFrame& frame = animation->get_frame(data.current_animation.view(), data.frame);
	data.remain = frame.duration;
}

void AnimatedSprite::render(const Transform2D& transform)
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
        transform, current_frame.sprite, 
        rect, src_rect, color, flags
    );
}


void AnimatedSprite::play(StringView animation_name)
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
}

void AnimatedSprite::stop()
{
	data.playing = false;
	data.frame = 0;
	data.current_animation.set("");
}


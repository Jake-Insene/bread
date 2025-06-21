#include "2d/animated_sprite_2d.h"

#include "graphics/graphics.h"


void AnimatedSprite2D::init(const CreateInfo&)
{
	mark(MARK_RENDER);
	data.current_animation = String::with_allocator(allocator);
}

void AnimatedSprite2D::deinit()
{
	data.current_animation.destroy();
}

void AnimatedSprite2D::internal_update(f64 dt)
{
	data.remain -= dt;

	if (data.remain < 0)
	{
		data.frame += 1;
		if (data.frame >= animation->get_frame_count(data.current_animation.view()))
		{
			data.frame = 0;

			SpriteAnimation::Animation& anim = animation->get_animation(data.current_animation.view());
			if (!anim.loop)
			{
				data.playing = false;
				unmark(MARK_INTERNAL_UPDATE);
			}
		}

		SpriteAnimation::SpriteFrame& frame = animation->get_frame(data.current_animation.view(), data.frame);
		data.remain = frame.duration;
	}
}

void AnimatedSprite2D::render()
{
	if (animation && data.playing)
	{
		SpriteAnimation::SpriteFrame& current_frame = animation->get_frame(
			data.current_animation.view(), data.frame
		);
		Vector2 extent = Vector2(current_frame.sprite->get_size());
		Rect2D src_rect{Vector2(), extent};
		Graphics2D::draw_texture(
			get_global_transform(), extent, extent,
			src_rect, current_frame.sprite->texture_id,
			Color(255, 255, 255, 255), (RenderCommand::SpriteFlags)_get_render_flags()
		);
	}
}

void AnimatedSprite2D::play(StringView anim)
{
	if (!animation || !animation->has_animation(anim))
		return;

	data.playing = true;
	data.frame = 0;

	data.current_animation.set(anim);
	data.remain = animation->get_frame(anim, 0).duration;
	mark(MARK_INTERNAL_UPDATE);
}

void AnimatedSprite2D::stop()
{
	data.playing = false;
	data.frame = 0;
	data.current_animation.set("");
	unmark(MARK_INTERNAL_UPDATE);
}


u32 AnimatedSprite2D::_get_render_flags()
{
	u32 flags = RenderCommand::FLAG_SPRITE_NONE;
	flags |= centered ? RenderCommand::FLAG_SPRITE_NONE : RenderCommand::FLAG_SPRITE_TOP_LEFT;
	flags |= flip_v ? RenderCommand::FLAG_SPRITE_FLIP_V : RenderCommand::FLAG_SPRITE_NONE;
	flags |= flip_h ? RenderCommand::FLAG_SPRITE_FLIP_H : RenderCommand::FLAG_SPRITE_NONE;
	return flags;
}
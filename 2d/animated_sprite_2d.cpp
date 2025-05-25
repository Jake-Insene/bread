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
		Graphics::add_cmd(
			RenderCommand
			{
				.type = RenderCommand::DRAW_SPRITE,
				.sprite =
				{
					.transform = get_global_transform(),
					.texture_extent = extent,
					.dest_extent = extent,
					.src_rect = src_rect,
					.texture = current_frame.sprite->texture_id,
					.color = Color(255, 255, 255, 255),
					.flags = (RenderCommand::SpriteFlags)_get_render_flags(),
				},
			}
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
	u32 flags = RenderCommand::FLAG_NONE;
	flags |= centered ? RenderCommand::FLAG_NONE : RenderCommand::FLAG_TOP_LEFT;
	flags |= flip_v ? RenderCommand::FLAG_FLIP_V : RenderCommand::FLAG_NONE;
	flags |= flip_h ? RenderCommand::FLAG_FLIP_H : RenderCommand::FLAG_NONE;
	return flags;
}
#pragma once
#include "core/header.h"
#include "io/resource.h"
#include "io/texture.h"

struct SpriteAnimation : Resource
{
	RESOURCE(RESOURCE_SPRITE_ANIMATION, .LoadFromAssets = false);

	struct SpriteFrame
	{
		Texture2D* sprite;
		f32 duration;
	};

	struct Animation
	{
		Array<SpriteFrame> frames;
		bool loop;
	};

	StringMap<Animation> animations;

	void destroy();

	void add_animation(StringView anim_name, Slice<SpriteFrame> frames, bool loop);
	[[nodiscard]] bool has_animation(StringView anim_name) const { return animations.has(anim_name); }
	[[nodiscard]] Animation& get_animation(StringView anim_name) { return animations.get(anim_name); }

	[[nodiscard]] SpriteFrame& get_frame(StringView anim_name, i32 index);
	[[nodiscard]] i32 get_frame_count(StringView anim_name);
};
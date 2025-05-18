#include "io/sprite_animation.h"

#include "io/resource_manager.h"


void SpriteAnimation::destroy()
{
	Resource::destroy();
	for (auto& it : animations)
	{
		it.second.frames.destroy();
	}

	animations.destroy();
}


void SpriteAnimation::add_animation(StringView anim_name, Slice<SpriteFrame> frames, bool loop)
{
	Animation& anim = animations.insert(anim_name, Animation());
	anim.loop = loop;
	anim.frames = Array<SpriteFrame>::from_items(ResourceManager::get_allocator(), frames);
}

SpriteAnimation::SpriteFrame& SpriteAnimation::get_frame(StringView anim_name, i32 index)
{
	DebugAssert(animations.has(anim_name), "the animation doesn't exists");
	Animation& anim = animations.get(anim_name);
	DebugAssert(index < i32(anim.frames.count), "invalid frame index");
	return anim.frames[index];
}

i32 SpriteAnimation::get_frame_count(StringView anim_name)
{
	DebugAssert(animations.has(anim_name), "the animation doesn't exists");
	Animation& anim = animations.get(anim_name);
	return anim.frames.count;
}

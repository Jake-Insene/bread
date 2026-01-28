#include "resource/sprite_animation.h"

#include "resource/resource_manager.h"


void SpriteAnimation::init()
{
	Resource::init(RESOURCE_SPRITE_ANIMATION);
	auto allocator = ResourceManager::get_allocator();

	animations = StringMap<SpriteAnimation::Animation>::with_allocator(allocator);
}

void SpriteAnimation::destroy()
{
	Resource::destroy();
	for (auto& it : animations.iter())
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
	return anim.frames.get(index);
}

usize SpriteAnimation::get_frame_count(StringView anim_name)
{
	DebugAssert(animations.has(anim_name), "the animation doesn't exists");
	Animation& animation = animations.get(anim_name);
	return animation.frames.count;
}

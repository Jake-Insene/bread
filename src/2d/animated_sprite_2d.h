#pragma once
#include "2d/object_2d.h"
#include "resource/sprite_animation.h"


struct AnimatedSprite2D : Object2D
{
	OBJECT(AnimatedSprite2D, Object2D);

    struct InternalData
    {
        i32 frame = 0;
        bool playing = false;
        String current_animation{};
        f32 remain;
    } data;
	
	SpriteAnimation* animation;

    bool centered = true;
    bool flip_h = false;
    bool flip_v = false;

	void init(const CreateInfo&);
    void deinit();

    void internal_update(f32 dt);
	void render();

    void play(StringView anim);
    void stop();
};
#pragma once
#include "2d/object_2d.h"
#include "resource/sprite_animation.h"


struct AnimatedSprite2D : Object2D
{
	OBJECT(AnimatedSprite2D, Object2D);

	// As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        i32 frame = 0;
        bool playing = false;
        String current_animation{};
        f32 remain;
    } data;
	
	SpriteAnimation* animation;

    bool centered = true;
    bool flip_v = false;
    bool flip_h = false;

	void init(const CreateInfo&);
    void deinit();

    void internal_update(f64 dt);
	void render();

    void play(StringView anim);
    void stop();

    u32 _get_render_flags();
};
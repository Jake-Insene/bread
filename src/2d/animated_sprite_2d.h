#pragma once
#include "2d/object_2d.h"
#include "resource/sprite_animation.h"


struct AnimatedSprite2D : Object2D
{
	OBJECT(AnimatedSprite2D, Object2D);

    struct InternalData
    {
        u32 frame = 0;
        bool playing = false;
        String current_animation{};
        f32 remain;
    } data;
	
    /*
    * Sprite modulate color.
    */
    Color color{255, 255, 255, 255};
    /*
    * Contains the animations and texture references to draw.
    */
	SpriteAnimation* animation;

    /*
    * If true the sprite is drawed with its center at the transform position,
    * otherwise the sprite top left will be at the transform position.
    */
    bool centered = true;
    /*
    * Flip the entire sprite horizontally
    */
    bool flip_h = false;
    /*
    * Flip the entire sprite vertically
    */
    bool flip_v = false;

	void init(const CreateInfo&);
    void deinit();

    void internal_update(f32 dt);
	void render();

    /*
    * Start playing the give animation.
    * 
    * @param animation_name The name of the animation to play.
    */
    void play(StringView animation_name) Function(FunctionNormal);
    /*
    * Stop the current playing animation.
    */
    void stop() Function(FunctionNormal);
};
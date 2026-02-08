#pragma once
#include "collections/string.h"
#include "modifier/core/renderable.h"


struct SpriteAnimation;


struct [[nodiscard]] AnimatedSprite : Renderable
{
    struct InternalData
    {
        u32 frame;
        bool playing;
        String current_animation;
        f32 remain;
    } data;
	
    /*
    * Sprite modulate color.
    */
    Color color;

    /*
    * Contains the animations and texture references to draw.
    */
	SpriteAnimation* animation;

    /*
    * If true the sprite is drawed with its center at the transform position,
    * otherwise the sprite top left will be at the transform position.
    */
    bool centered;
    /*
    * Flip the entire sprite horizontally
    */
    bool flip_h;
    /*
    * Flip the entire sprite vertically
    */
    bool flip_v;

    void init(const mem::Allocator& allocator);
    void deinit();

    void update(f32 dt);
	void render(RenderItemID render_item, const Transform2D& transform);

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

#pragma once
#include "display/window.h"
#include "io/texture.h"
#include "mem/generic_allocator.h"


// Application configuration
#define ENGINE_CONFIGURATION(...) EngineConfiguration __configuration__ = {__VA_ARGS__};
#define ENGINE_DEFAULT_CONFIGURATION(main_scene) \
    EngineConfiguration __configuration__ =\
    {\
        .CreateMainScene = []() -> Object* { return CreateObject<main_scene>(); },\
    }

// Preloading stage
#define PRELOAD_BEGIN() void __preload__()\
    {

#define PRELOAD_END() }

// Resource Loading
// SpriteAnimation
#define SPRITE_ANIMATION_BEGIN(name) {\
    SpriteAnimation* __sprite_animation__ = ResourceManager::create_sprite_animation(name);\

#define SPRITE_ANIMATION_END() }

#define ANIMATION_BEGIN(animation_name, loop) {\
    StringView __animation_name__ = animation_name; bool __loop__ = loop;\
    SpriteAnimation::SpriteFrame __frames__[] = {

#define ANIMATION_END() }; __sprite_animation__->add_animation(__animation_name__, __frames__, __loop__); }

#define ANIMATION_FRAME(texture_path, duration) SpriteAnimation::SpriteFrame(GetResource<Texture2D>(texture_path), duration),


// TileSet
#define TILE_SET_BEGIN(set_name, texture_path, tile_size, ...) {\
    TileSet* __tile_set__ = ResourceManager::create_tile_set(set_name, tile_size);\
    __tile_set__->texture = GetResource<Texture2D>(texture_path);

#define TILE_SET_END() }

#define TILE_SET_DATA_BEGIN(...) {\
    TileSet::TileData tiles_data[] = {

#define TILE_SET_DATA_END() }; __tile_set__->add_tiles_data(tiles_data); }

#define TILE_SET_TILE_BEGIN(...) {\
    TileSet::Tile tiles[] = {

#define TILE_SET_TILE_END() }; __tile_set__->add_tiles(tiles); }


#define TILE(position, data_index, ...) TileSet::Tile(position, data_index),
#define TILE_DATA(texture_position, ...) TileSet::TileData(texture_position),


struct Object;
struct InputEvent;

struct EngineConfiguration
{
    Vector2I WindowSize = Vector2I(Display::DefaultWidth, Display::DefaultHeight);
    Vector2I DisplayTargetSize = Vector2I(Display::DefaultWidth, Display::DefaultHeight);
    bool VSync = true;
    Object* (*CreateMainScene)();
};

struct Engine
{
    struct VTable
    {
    };

    struct InternalData
    {
        mem::GenericAllocator allocator;
        
        Window main_window;

        Texture* white_texture;

        i32 fps;
    };

    static inline InternalData data;
    
    static void initialize();
    static void shutdown();
    
    static void recreate_window();
    static void destroy();
    
    static void step();
    static void handle_input(const InputEvent& event);

    // Utility functions
    static i32 get_fps() { return data.fps; }

    static Window get_main_window() { return data.main_window; }

    static void set_vsync(bool vsync);
};

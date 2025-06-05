#pragma once
#include "display/window.h"
#include "resource/texture.h"
#include "mem/generic_allocator.h"

// Application configuration
#define EngineConfiguration(...) EngineConfiguration __configuration__ = {__VA_ARGS__};
#define DefaultCreateScene(name) []() -> Object* { return CreateObject<name>(); }
#define EngineDefaultConfiguration(main_scene) \
    EngineConfiguration __configuration__ =\
    {\
        .CreateMainScene = DefaultCreateScene(main_scene),\
    }

// Preloading stage
#define PreloadBegin() void __preload__()\
    {

#define PreloadEnd() }

// Resource Loading
// SpriteAnimation
#define SpriteAnimationBegin(name) {\
    SpriteAnimation* __sprite_animation__ = ResourceManager::create_sprite_animation(name);\

#define SpriteAnimationEnd() }

#define AnimationBegin(animation_name, loop) {\
    StringView __animation_name__ = animation_name; bool __loop__ = loop;\
    SpriteAnimation::SpriteFrame __frames__[] = {

#define AnimationEnd() }; __sprite_animation__->add_animation(__animation_name__, __frames__, __loop__); }

#define AnimationFrame(texture_path, duration) SpriteAnimation::SpriteFrame(GetResource<Texture2D>(texture_path), duration),


// TileSet
#define TileSetBegin(set_name, texture_path, tile_size, ...) {\
    TileSet* __tile_set__ = ResourceManager::create_tile_set(set_name, tile_size);\
    __tile_set__->texture = GetResource<Texture2D>(texture_path);

#define TileSetEnd() }

#define TileSetDataBegin(...) {\
    TileSet::TileData tiles_data[] = {

#define TileSetDataEnd() }; __tile_set__->add_tiles_data(tiles_data); }

#define TileSetTileBegin(...) {\
    TileSet::Tile tiles[] = {

#define TileSetTileEnd() }; __tile_set__->add_tiles(tiles); }


#define DefineTile(position, data_index, ...) TileSet::Tile(position, data_index),
#define DefineTileData(texture_position, ...) TileSet::TileData(texture_position),


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
        bool recreate_requested;
    };

    static inline InternalData data;
    
    static void initialize();
    static void shutdown();
    
    static void recreate_window();
    static void destroy();
    
    static void step();
    static void handle_input(const InputEvent& event);

    static void request_recreate_window();

    // Utility functions
    static i32 get_fps() { return data.fps; }

    static Window get_main_window() { return data.main_window; }

    static void set_vsync(bool vsync);
};

#pragma once
#include "collections/job_queue.h"
#include "display/window.h"
#include "resource/texture.h"
#include "mem/generic_allocator.h"

// Application configuration
#define EngineConfiguration(...) EngineConfiguration __configuration__ = {__VA_ARGS__};
#define DefaultCreateScene(name) []() -> Object* { return Object::create<name>(); }
#define EngineDefaultConfiguration(main_scene) \
    EngineConfiguration __configuration__ =\
    {\
        .create_main_scene = DefaultCreateScene(main_scene),\
    }

// Preloading stage
#define PreloadBegin() void __preload__()\
    {

#define PreloadEnd() }

// Resource Loading
#define PreloadTexture(path)

// SpriteAnimation
#define SpriteAnimationBegin(name) {\
    SpriteAnimation* __sprite_animation__ = ResourceManager::create_sprite_animation(name);\

#define SpriteAnimationEnd() }

#define AnimationBegin(animation_name, loop) {\
    StringView __animation_name__ = animation_name; bool __loop__ = loop;\
    SpriteAnimation::SpriteFrame __frames__[] = {

#define AnimationEnd() }; __sprite_animation__->add_animation(__animation_name__, __frames__, __loop__); }

#define AnimationFrame(texture_path, duration) SpriteAnimation::SpriteFrame(Resource::load<Texture2D>(texture_path), duration),


// TileSet
#define TileSetBegin(set_name, texture_path, tile_size, ...) {\
    TileSet* __tile_set__ = ResourceManager::create_tile_set(set_name, tile_size);\
    __tile_set__->set_texture(Resource::load<Texture2D>(texture_path));

#define TileSetEnd() }

#define TileSetDataBegin(...) {\
    TileSet::TileData tiles_data[] = {

#define TileSetDataEnd() }; __tile_set__->add_tiles_data(tiles_data); }

#define TileSetTileBegin(...) {\
    TileSet::Tile tiles[] = {

#define TileSetTileEnd() }; __tile_set__->add_tiles(tiles); }


#define TileSetTile(position, data_index, has_shape, ...) TileSet::Tile(position, data_index, has_shape),
#define TileSetTileData(texture_position, ...) TileSet::TileData(texture_position),


struct Object;
struct InputEvent;

struct EngineConfiguration
{
    Vector2I viewport_size = Vector2I(Display::DefaultWidth, Display::DefaultHeight);
    Object* (*create_main_scene)();
    bool vsync = true;
    bool keep_viewport;
    bool enable_debug_console = false;
};

extern EngineConfiguration __configuration__;
extern void __preload__();

struct Engine
{
    static constexpr usize DefaultMainQueueSize = 16;

    struct VTable
    {
    };

    struct InternalData
    {
        mem::GenericAllocator allocator;
        JobQueue main_queue;
        
        Window main_window;

        Texture* white_texture;

        i32 fps;
        bool vsync_cache;
    };

    static inline InternalData data;
    
    static void initialize();
    static void shutdown();
    
    static void destroy();
    
    static void step();
    static void handle_input(const InputEvent& event);

    static void request_recreate_window();

    // Utility functions
    static i32 get_fps() { return data.fps; }

	static EngineConfiguration& get_configuration() { return __configuration__; }

    static Window get_main_window() { return data.main_window; }

    static void set_vsync(bool vsync);
    static bool get_vsync() { return data.vsync_cache; }

    template<typename Fn> 
    static void add_main_job(Fn fn)
    {
        data.main_queue.add_job(fn);
    }
};

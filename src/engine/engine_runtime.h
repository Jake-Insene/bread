#pragma once
#include "concurrency/job_queue.h"
#include "display/window.h"
#include "engine/configuration.h"
#include "mem/generic_allocator.h"
#include "resource/texture.h"
#include "resource/resource_manager.h"
#include "resource/sprite_animation.h"
#include "resource/tile_set.h"
#include "resource/material.h"
#include "systems/system_manager.h"


// Preloading stage
#define PreloadBegin() void __preload__()\
    {

#define PreloadEnd() }

// Resource Loading
#define PreloadTexture(path) (void)Resource::load<Texture2D>(path)
#define PreloadMaterial(path) (void)Resource::load<Material>(path)

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

struct InputEvent;
struct EngineConfiguration;

extern EngineConfiguration __configuration__;
extern void __preload__();

struct EngineRuntime
{
    static constexpr usize DefaultMainQueueSize = 16;
    
    mem::GenericAllocator allocator;
    
    SystemManager system_manager;

    Graphics::PhysicalDeviceID physical_device;

    JobQueue main_queue;
        
    Window main_window;

    Texture* white_texture;

    i32 fps;
    bool vsync_cache;

    void init();
    void destroy();
    
    void step();
    void handle_event(const InputEvent& event);

    void request_recreate_window();

    // Utility functions
    SystemManager& get_system_manager() { return system_manager; }

    Graphics::PhysicalDeviceID get_selected_gpu_device() { return physical_device; }

    i32 get_fps() { return fps; }

	EngineConfiguration& get_configuration() { return __configuration__; }

    Window get_main_window() { return main_window; }

    void set_vsync(bool vsync);
    bool get_vsync() { return vsync_cache; }

    template<typename Fn> 
    void add_main_job(Fn fn)
    {
        main_queue.add_job(fn);
    }

    void _select_physical_device();
};

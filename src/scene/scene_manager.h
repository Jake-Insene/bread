#pragma once
#include "collections/array.h"
#include "collections/hash_map.h"
#include "math/color.h"
#include "math/vec2.h"
#include "render/viewport.h"
#include "scene/scene.h"



struct SceneManager
{
    struct QueueFreeInfo
    {
        Scene* child;
    };

    struct InternalData
    {
        mem::Allocator allocator;
    
        bool keep_viewport;
        
        Vector2I viewport_size;
        Viewport main_viewport;

        Scene* current_scene;
    
        struct
        {
            f64 internal_update_time;
            f64 update_time;
            f64 physics_2d_time;
            f64 render_time;
            f64 render_scene_time;
            f64 present_scene_time;
        } debug_time;

        f32 last_time;
        f32 time_acum;
        f32 delta_time;
    
        i32 fps_counter;
        i32 fps_acum;

        struct
        {
            bool requested;
            Scene* new_scene;
        } change_scene;

        HashMap<Scene*, QueueFreeInfo> queue_frees;
    };
    
    static inline InternalData data;
    
    static f32 get_delta_time() { return data.delta_time; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();
    
    static void change_scene(Scene* new_scene);
    
    static void step();

    static void recreate_window();

    static void set_keep_viewport(bool keep_viewport);
    static bool get_keep_viewport() { return data.keep_viewport; }

    static Viewport& get_main_viewport() { return data.main_viewport; }
    static void set_viewport_size(const Vector2I& new_vp_size);
    static Vector2I get_viewport_size() { return data.viewport_size; }

    static void set_background_color(Color new_bg_color) { get_main_viewport().clear_color = new_bg_color; }
    static Color get_background_color() { return get_main_viewport().clear_color; }

    static void scene_handle_event(const InputEvent& event);
    
    static void _render_manager_tick();

    static void _handle_change_scene();

    static Vector2 _screen_make_local_to_canvas(const Vector2& pos);

    static void _remove_object_from_list(Scene* scene);
    static void _handle_object_mark_changed();
    static void _queue_free(Scene* parent, Scene* scene);
    static void _update_object_mark(Scene::MarkName mark_name, Scene* scene, bool marked);
};

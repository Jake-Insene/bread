#pragma once
#include "collections/array.h"
#include "collections/hash_map.h"
#include "math/color.h"
#include "math/vec2.h"
#include "scene/scene.h"



struct SceneManager
{
    struct QueueFreeInfo
    {
        Scene* child;
    };

    mem::Allocator* allocator;

    bool keep_viewport;
    
    Vector2I viewport_size;
    //Viewport main_viewport;

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
    } change_scene_info;
    
    f32 get_delta_time() { return delta_time; }

    void initialize(mem::Allocator* _allocator);
    void shutdown();
    
    void change_scene(Scene* new_scene);
    
    void step();

    void recreate_window();

    void set_keep_viewport(bool _keep_viewport);
    bool get_keep_viewport() { return keep_viewport; }

    void set_viewport_size(const Vector2I& new_vp_size);
    Vector2I get_viewport_size() { return viewport_size; }

    void scene_handle_event(const InputEvent& event);
    
    void _render_manager_tick();

    void _handle_change_scene();

    Vector2 _screen_make_local_to_canvas(const Vector2& pos);

    void _remove_object_from_list(Scene* scene);
    void _handle_object_mark_changed();
    void _queue_free(Scene* parent, Scene* scene);
    void _update_object_mark(Scene::MarkName mark_name, Scene* scene, bool marked);
};

#pragma once
#include "collections/array.h"
#include "collections/singleton.h"
#include "math/color.h"
#include "math/vec2.h"
#include "object/object.h"
#include "render/viewport.h"


struct CanvasObject;
struct Camera2D;


struct SceneManager
{
    struct QueueFreeInfo
    {
        Object* parent;
        Object* child;
    };

    struct MarkChangedInfo
    {
        MarkName mark_name;
        Object* object;
        bool marked;
    };

    struct InternalData
    {
        mem::Allocator allocator;
    
        bool keep_viewport;
        
        Vector2I viewport_size;
        Viewport main_viewport;

        Object* current_scene;
        Camera2D* current_camera;
    
        struct
        {
            f64 internal_update_time;
            f64 update_time;
            f64 physics_2d_time;
            f64 render_time;
            f64 driver_render_time;
            f64 driver_present_time;
        } debug_time;

        f32 last_time;
        f32 time_acum;
        f32 delta_time;
    
        i32 fps_counter;
        i32 fps_acum;

        struct
        {
            bool requested;
            Object* new_scene;
        } change_scene;

        Array<CanvasObject*> root_canvas;
        Array<CanvasObject*> touched_focus;
        HashMap<ObjectID, QueueFreeInfo> queue_frees;
        Array<Object*> int_update_list;
        Array<Object*> update_list;
        Array<Object*> render_list;
        Array<MarkChangedInfo> objects_mark_changed;
    };
    
    static inline InternalData data;
    
    static f32 get_delta_time() { return data.delta_time; }

    static void initialize(const mem::Allocator& allocator);
    static void shutdown();
    
    static void change_scene(Object* new_scene);
    
    static void step();

    static void recreate_window();

    static void set_keep_viewport(bool keep_viewport);
    static bool get_keep_viewport() { return data.keep_viewport; }

    static Viewport& get_main_viewport() { return data.main_viewport; }
    static void set_viewport_size(const Vector2I& new_vp_size);
    static Vector2I get_viewport_size() { return data.viewport_size; }

    static void set_background_color(Color new_bg_color) { get_main_viewport().clear_color = new_bg_color; }
    static Color get_background_color() { return get_main_viewport().clear_color; }

    static void set_camera_2d(Camera2D* camera);
    [[nodiscard]] static Camera2D* get_camera_2d() { return data.current_camera; }

    static void scene_handle_input(const InputEvent& event);
    
    static void _try_clear_root_canvas();

    static void _handle_change_scene();

    static Vector2 _screen_make_local_to_canvas(const Vector2& pos);
    static CanvasObject* _find_canvas_in_pos(const Vector2& pos);

    static void _add_root_canvas(CanvasObject* c);
    
    static void _remove_object_from_list(Object* object);
    static void _handle_object_mark_changed();
    static void _queue_free(Object* parent, Object* child);
    static void _update_object_mark(MarkName mark_name, Object* object, bool marked);
};

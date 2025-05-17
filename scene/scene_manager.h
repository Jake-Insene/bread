#pragma once
#include "graphics/render_target.h"
#include "object/object.h"
#include "math/color.h"
#include "math/vec2.h"
#include "mem/generic_allocator.h"

struct Control;
struct Camera2D;


struct SceneManager
{
    struct QueueFreeInfo
    {
        Object* parent;
        Object* child;
    };

    struct InternalData
    {
        mem::Allocator allocator;
        RenderTarget display_target;
    
        Color clear_color;
        Object* current_scene;
        Camera2D* current_camera;
    
        f64 last_time;
        f64 time_acum;
        f64 delta_time;
    
        i32 fps_counter;
        i32 fps_acum;
        
        Array<Control*> gui_roots;
        Array<Control*> touched_focus;
        HashMap<ObjectID, QueueFreeInfo> queue_frees;
    };
    
    static inline InternalData data;
    
    static f64 get_delta_time() { return data.delta_time; }
    static RenderTarget get_display_target() { return data.display_target; }
    
    static void initialize(mem::Allocator allocator);
    static void shutdown();
    
    static void change_scene(Object* new_scene);
    
    static void step();

    static Vector2 _screen_make_local(Vector2 pos);
    static Control* _find_control_in_pos(Vector2 pos);
    static void _handle_input(const InputEvent& event);

    static void set_camera_2d(Camera2D* camera);
    [[nodiscard]] static Camera2D* get_camera_2d() { return data.current_camera; }

    static void _add_root_control(Control* c);
    
    static void _queue_free(Object* parent, Object* child);
};

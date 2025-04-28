#pragma once
#include "graphics/render_target.h"
#include "objects/object.h"
#include "math/color.h"
#include "math/vec2.h"
#include "mem/generic_allocator.h"

struct Control;
struct Camera2D;


struct SceneManager
{
    struct InternalData
    {
        mem::Allocator allocator;
        RenderTarget display_target;
        Vector2I window_size;
    
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
    };
    
    static constexpr i32 DefaultWidth = 600;
    static constexpr i32 DefaultHeight = 800;
    
    static inline InternalData data;
    
    static f64 get_delta_time() { return data.delta_time; }
    
    static void initialize(mem::Allocator allocator);
    static void shutdown();
    
    static void change_scene(Object* new_scene);
    
    static void step();
    
    static Vector2 screen_make_local(Vector2 pos);
    static Control* find_control_in_pos(Vector2 pos);
    static void handle_input(const InputEvent& event);

    static void set_camera_2d(Camera2D* camera);
    
    static void add_root_control(Control* c);
};

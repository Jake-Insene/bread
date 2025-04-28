#include "objects/scene_manager.h"

#include "core/time.h"
#include "core/event.h"
#include "debug/debug.h"
#include "engine/engine.h"
#include "graphics/graphics.h"
#include "graphics/egl/egl.h"
#include "gui/control.h"
#include "2d/camera_2d.h"
#include "objects/object_allocator.h"
#include "physics/physics_2d.h"


void SceneManager::initialize(mem::Allocator allocator)
{
    data.allocator = allocator;

    data.display_target = RenderTarget::create(
        Vector2I(DefaultWidth, DefaultHeight)
    );
    data.window_size = EGL::get_surface_size();

    data.clear_color = {0, 0, 0, 255};
    
    data.current_scene = nullptr;
    data.current_camera = nullptr;
    
    data.last_time = Time::get_time();
    data.time_acum = 0;
    data.delta_time = 0;
    
    data.fps_counter = 0;
    data.fps_acum = 0;
    
    data.gui_roots = Array<Control*>::with_size(
        data.allocator, 4
    );
    
    data.touched_focus = Array<Control*>::with_size(
        data.allocator, 4
    );

	// To avoid any Out Of Range error in handle_input
    data.touched_focus.resize(1);
}

void SceneManager::shutdown()
{
    if(data.current_scene)
    {
		ObjectCallRef(data.current_scene, exit);
        DestroyObject(data.current_scene);
    }

    data.touched_focus.destroy();
    data.gui_roots.destroy();
}

void SceneManager::change_scene(Object* new_scene)
{
    DebugAssert(new_scene != nullptr, "new scene can't be null");
    if(data.current_scene)
    {
        ObjectCallRef(data.current_scene, exit);
        DestroyObject(data.current_scene);
    }
    
    data.current_scene = new_scene;
    ObjectCallRef(data.current_scene, start);
}

void SceneManager::step()
{
    if(data.current_scene)
    {
        f64 current = Time::get_time();
        data.delta_time = current - data.last_time;
        data.last_time = current;
        
        data.time_acum += data.delta_time;
        if(data.time_acum >= 1.0)
        {
            data.fps_counter = data.fps_acum;
            Engine::data.fps = data.fps_counter;
            Debug::info("FPS: %i, Avg Frame Time: %f", data.fps_counter, data.delta_time);
            
            data.fps_acum = 0;
            data.time_acum = 0;
        }
        
        data.current_scene->handle_internal_update(data.delta_time);
        data.current_scene->handle_update(data.delta_time);
        
        Graphics::add_cmd(
            RenderCommand
            {
                .type = RenderCommand::BIND_RENDER_TARGET,
                .bind = 
                {
                    .source_id = data.display_target.render_target_id,
                },
            }
        );

        Graphics::add_cmd(
            RenderCommand
            {
                .type = RenderCommand::CLEAR_RENDER_TARGET,
                .clear =
                {
                    .rid = data.display_target.render_target_id,
                    .color = data.clear_color,
                },
            }
        );

        Physics2D::step(data.delta_time);

        data.current_scene->handle_render();
        
        Graphics::render();
        Graphics::present();
        
        data.fps_acum++;
    }
}

Vector2 SceneManager::screen_make_local(Vector2 pos)
{
    // converting touch position into local scene position
    Vector2I scene_size = data.display_target.get_size();
            
    f32 normalized_x = pos.x / (f32)data.window_size.x;
    f32 normalized_y = pos.y / (f32)data.window_size.y;
            
    // Setting new position
    return Vector2(
        normalized_x * (f32)scene_size.x,
        normalized_y * (f32)scene_size.y
    );
}

Control* SceneManager::find_control_in_pos(Vector2 pos)
{
    for(auto& c : data.gui_roots)
    {
        if(ObjectCallRef(c, is_inside, pos))
        {
            return c;
        }
    }
    
    return nullptr;
}

void SceneManager::handle_input(const InputEvent& event)
{
    if(data.current_scene && data.current_scene->can_handle_event())
    {
        if(event.type == INPUT_EVENT_TOUCH)
        {
            auto& et = event.get<InputEventTouch>();
            InputEventTouch new_event = et;
            new_event = et;
            
            new_event.position = screen_make_local(et.position);
            data.touched_focus.resize(et.pointer+1);
            if(Control* c = find_control_in_pos(new_event.position))
            {
                data.touched_focus[et.pointer] = c;
                ObjectCallRef(c, event, new_event);
            }
            else
            {
                c = data.touched_focus[et.pointer];
                if(c)
                {
                    // is_in_area will be always false
                    new_event.pressed = false;
                    ObjectCallRef(c, event, new_event);
                }
                data.touched_focus[et.pointer] = nullptr;
            }
            
            ObjectCallRef(data.current_scene, event, new_event);
        }

        if (event.type == INPUT_EVENT_MOUSE_BUTTON)
        {
            auto& et = event.get<InputEventMouseButton>();
            InputEventMouseButton new_event = et;
            new_event = et;

            new_event.position = screen_make_local(et.position);
            if (Control* c = find_control_in_pos(new_event.position))
            {
                data.touched_focus[0] = c;
                ObjectCallRef(c, event, new_event);
            }
            else
            {
                c = data.touched_focus[0];
                if (c)
                {
                    // is_in_area will be always false
                    new_event.left = false;
                    ObjectCallRef(c, event, new_event);
                }
                data.touched_focus[0] = nullptr;
            }

            ObjectCallRef(data.current_scene, event, new_event);
        }
    }
}

void SceneManager::set_camera_2d(Camera2D* camera)
{
    data.current_camera = camera;
}

void SceneManager::add_root_control(Control* c)
{
    (void)data.gui_roots.add(c);
}

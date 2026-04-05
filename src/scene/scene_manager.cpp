#include "scene/scene_manager.h"

#include "debug/profiler.h"
#include "engine/engine.h"
#include "input/input.h"
#include "log/log.h"
#include "mem/utils.h"
#include "physics/physics_2d.h"


void SceneManager::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    data.current_scene = nullptr;
    
    data.last_time = f32(OS::get_time());
    data.time_acum = 0;
    data.delta_time = 0;
    
    data.fps_counter = 0;
    data.fps_acum = 0;
    
    // TODO: touch focus.
	// To avoid any out of range error in handle_input
    // data.touched_focus.resize(1);
}

void SceneManager::shutdown()
{
    if(data.current_scene)
    {
		SceneCallRef(data.current_scene, on_exit);
		SceneCallRef(data.current_scene, on_destroy);
        data.allocator.free(mem::to_bytes(Slice<Scene>(data.current_scene, 1)));
    }
}

void SceneManager::change_scene(Scene* new_scene)
{
    DebugAssert(new_scene != nullptr, "new scene can't be null");
    DebugAssert(data.change_scene.requested == false, "a change scene was already requested");

    if (data.current_scene == nullptr)
    {
        data.current_scene = new_scene;
        SceneCallRef(data.current_scene, on_enter);
        return;
    }

    data.change_scene.requested = true;
    data.change_scene.new_scene = new_scene;
}

void SceneManager::step()
{
    if (data.current_scene == nullptr)
        return;

    _handle_change_scene();

    f32 current = f32(OS::get_time());
    data.delta_time = current - data.last_time;
    data.last_time = current;

    data.time_acum += data.delta_time;
    if (data.time_acum >= 1.0)
    {
        data.fps_counter = data.fps_acum;
        Engine::local_data.engine_runtime->fps = data.fps_counter;
        Log::info(
            "Frame Info: FPS: {}\n"
            "\tAvg Frame Time: {}\n"
            "\tInternal Update Time: {}\n"
            "\tUpdate Time: {}\n"
            "\tPhysics 2D Time: {}\n"
            "\tRender Time: {}\n"
            "\tRender Scene: {}\n"
            "\tPresent Scene Time: {}",
            data.fps_counter, data.delta_time, data.debug_time.internal_update_time,
            data.debug_time.update_time, data.debug_time.physics_2d_time, 
            data.debug_time.render_time, data.debug_time.render_scene_time,
            data.debug_time.present_scene_time
        );

        data.fps_acum = 0;
        data.time_acum = 0;
    }

    {
        PROFILE_SCOPE(
            data.debug_time.internal_update_time = duration;
        );
     
        SceneCallRef(data.current_scene, on_internal_update, data.delta_time);
    }

    {
        PROFILE_SCOPE(
            data.debug_time.update_time = duration;
        );

        SceneCallRef(data.current_scene, on_update, data.delta_time);
    }

    {
        PROFILE_SCOPE(
            data.debug_time.physics_2d_time = duration;
        );
        Physics2D::step(data.delta_time);
    }

    {
        PROFILE_SCOPE(
            data.debug_time.render_time = duration;
        );

        SceneCallRef(data.current_scene, on_render, data.delta_time);
    }

    if(!Engine::get_configuration().enable_custom_rendering)
    {
        _render_manager_tick();
    }

    data.fps_acum++;

    _handle_object_mark_changed();
}

void SceneManager::recreate_window()
{
    if(!get_keep_viewport())
    {
        set_viewport_size(Engine::get_main_window().get_size());
    }
}

void SceneManager::set_keep_viewport(bool keep_viewport)
{
    if (data.keep_viewport == keep_viewport)
        return;

    data.keep_viewport = keep_viewport;
}

void SceneManager::set_viewport_size(const Vector2I& new_vp_size)
{
    if (data.viewport_size == new_vp_size)
        return;

    data.viewport_size = new_vp_size;
}

void SceneManager::scene_handle_event(const InputEvent& event)
{
    if (data.current_scene == nullptr)
        return;

    switch (event.type)
    {
    case InputEventType::Touch:
    {
        auto& et = event.get<InputEventTouch>();
        InputEventTouch new_event = et;
        new_event = et;

        new_event.position = _screen_make_local_to_canvas(et.position);
        SceneCallRef(data.current_scene, on_event, new_event);
    }
    break;
    case InputEventType::MouseButton:
    {
        auto& et = event.get<InputEventMouseButton>();
        InputEventMouseButton new_event = et;
        new_event = et;

        new_event.position = _screen_make_local_to_canvas(et.position);
        SceneCallRef(data.current_scene, on_event, new_event);
    }
    break;
    default:
    {
        SceneCallRef(data.current_scene, on_event, event);
    }
        break;
    }
}

void SceneManager::_render_manager_tick()
{
    {
        PROFILE_SCOPE(
            data.debug_time.render_scene_time = duration;
        );
    }

    {
        PROFILE_SCOPE(
            data.debug_time.present_scene_time = duration;
        );
    }
}

void SceneManager::_handle_change_scene()
{
    if (data.change_scene.requested == false)
        return;

    data.change_scene.requested = false;
    SceneCallRef(data.current_scene, on_exit);
    SceneCallRef(data.current_scene, on_destroy);
    data.allocator.free(mem::to_bytes(Slice<Scene>(data.current_scene, 1)));

    data.current_scene = data.change_scene.new_scene;
    data.change_scene.new_scene = nullptr;
    SceneCallRef(data.current_scene, on_enter);
}

Vector2 SceneManager::_screen_make_local_to_canvas(const Vector2& pos)
{
    // converting touch/mouse position into local canvas position
    const Vector2 window_size = Vector2(Engine::get_main_window().get_size());
    const Vector2 viewport_size = Vector2(get_viewport_size());

    // normalized position
    const Vector2 normalized_pos = pos / window_size;
    const Vector2 canvas_pos = normalized_pos * viewport_size;

    return canvas_pos;
}

void SceneManager::_remove_object_from_list(Scene*)
{}

void SceneManager::_handle_object_mark_changed()
{}

void SceneManager::_queue_free(Scene*, Scene*)
{}

void SceneManager::_update_object_mark(Scene::MarkName, Scene*, bool)
{}


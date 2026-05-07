#include "scene/scene_manager.h"

#include "debug/profiler.h"
#include "engine/engine.h"
#include "input/input.h"
#include "log/log.h"
#include "mem/utils.h"
#include "physics/physics_2d.h"


void SceneManager::initialize(const mem::Allocator& _allocator)
{
    allocator = _allocator;

    current_scene = nullptr;
    
    last_time = f32(OS::get_time());
    time_acum = 0;
    delta_time = 0;
    
    fps_counter = 0;
    fps_acum = 0;
    
    // TODO: touch focus.
	// To avoid any out of range error in handle_input
    // data.touched_focus.resize(1);
}

void SceneManager::shutdown()
{
    if(current_scene != nullptr)
    {
		SceneCallRef(current_scene, on_exit);
		SceneCallRef(current_scene, on_destroy);
        allocator.free(mem::to_bytes(Slice<Scene>(current_scene, 1)));
    }
}

void SceneManager::change_scene(Scene* new_scene)
{
    DebugAssert(new_scene != nullptr, "new scene can't be null");
    DebugAssert(change_scene_info.requested == false, "a change scene was already requested");

    if (current_scene == nullptr)
    {
        current_scene = new_scene;
        SceneCallRef(current_scene, on_enter);
        return;
    }

    change_scene_info.requested = true;
    change_scene_info.new_scene = new_scene;
}

void SceneManager::step()
{
    if (current_scene == nullptr)
    {
        return;
    }

    _handle_change_scene();

    f32 current = f32(OS::get_time());
    delta_time = current - last_time;
    last_time = current;

    time_acum += delta_time;
    if (time_acum >= 1.0)
    {
        fps_counter = fps_acum;
        Engine::local_data.engine_runtime->fps = fps_counter;
        Log::info(
            "Frame Info: FPS: {}\n"
            "\tAvg Frame Time: {}\n"
            "\tInternal Update Time: {}\n"
            "\tUpdate Time: {}\n"
            "\tPhysics 2D Time: {}\n"
            "\tRender Time: {}\n"
            "\tRender Scene: {}\n"
            "\tPresent Scene Time: {}",
            fps_counter, delta_time, debug_time.internal_update_time,
            debug_time.update_time, debug_time.physics_2d_time, 
            debug_time.render_time, debug_time.render_scene_time,
            debug_time.present_scene_time
        );

        fps_acum = 0;
        time_acum = 0;
    }

    {
        PROFILE_SCOPE(
            debug_time.internal_update_time = duration;
        );
     
        SceneCallRef(current_scene, on_internal_update, delta_time);
    }

    {
        PROFILE_SCOPE(
            debug_time.update_time = duration;
        );

        SceneCallRef(current_scene, on_update, delta_time);
    }

    {
        PROFILE_SCOPE(
            debug_time.physics_2d_time = duration;
        );
        Physics2D::step(delta_time);
    }

    {
        PROFILE_SCOPE(
            debug_time.render_time = duration;
        );

        SceneCallRef(current_scene, on_render, delta_time);
    }

    if(!Engine::get_configuration().enable_custom_rendering)
    {
        _render_manager_tick();
    }

    fps_acum++;

    _handle_object_mark_changed();
}

void SceneManager::recreate_window()
{
    if(!get_keep_viewport())
    {
        set_viewport_size(Engine::get_main_window().get_size());
    }
}

void SceneManager::set_keep_viewport(bool _keep_viewport)
{
    if (keep_viewport == _keep_viewport)
    {
        return;
    }

    keep_viewport = _keep_viewport;
}

void SceneManager::set_viewport_size(const Vector2I& new_vp_size)
{
    if (viewport_size == new_vp_size)
    {
        return;
    }

    viewport_size = new_vp_size;
}

void SceneManager::scene_handle_event(const InputEvent& event)
{
    if (current_scene == nullptr)
    {
        return;
    }

    switch (event.type)
    {
    case InputEventType::Touch:
    {
        const InputEventTouch& et = event.get<InputEventTouch>();
        InputEventTouch new_event = et;
        new_event = et;

        new_event.position = _screen_make_local_to_canvas(et.position);
        SceneCallRef(current_scene, on_event, new_event);
    }
    break;
    case InputEventType::MouseButton:
    {
        const InputEventMouseButton& et = event.get<InputEventMouseButton>();
        InputEventMouseButton new_event = et;
        new_event = et;

        new_event.position = _screen_make_local_to_canvas(et.position);
        SceneCallRef(current_scene, on_event, new_event);
    }
    break;
    default:
    {
        SceneCallRef(current_scene, on_event, event);
    }
        break;
    }
}

void SceneManager::_render_manager_tick()
{
    {
        PROFILE_SCOPE(
            debug_time.render_scene_time = duration;
        );
    }

    {
        PROFILE_SCOPE(
            debug_time.present_scene_time = duration;
        );
    }
}

void SceneManager::_handle_change_scene()
{
    if (!change_scene_info.requested)
    {
        return;
    }

    change_scene_info.requested = false;
    SceneCallRef(current_scene, on_exit);
    SceneCallRef(current_scene, on_destroy);
    allocator.free(mem::to_bytes(Slice<Scene>(current_scene, 1)));

    current_scene = change_scene_info.new_scene;
    change_scene_info.new_scene = nullptr;
    SceneCallRef(current_scene, on_enter);
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


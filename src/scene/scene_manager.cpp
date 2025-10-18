#include "scene/scene_manager.h"

#include "debug/debug.h"
#include "debug/profiler.h"
#include "engine/engine.h"
#include "graphics/viewport.h"
#include "canvas/canvas_object.h"
#include "input/input.h"
#include "2d/camera_2d.h"
#include "object/object_allocator.h"
#include "physics/physics_2d.h"


void SceneManager::initialize(const mem::Allocator& allocator)
{
    data.allocator = allocator;

    data.main_viewport = Viewport::create_from_render_target(allocator, RenderTarget::get_main_render_target());
    
    data.current_scene = nullptr;
    data.current_camera = nullptr;
    
    data.last_time = f32(OS::get_time());
    data.time_acum = 0;
    data.delta_time = 0;
    
    data.fps_counter = 0;
    data.fps_acum = 0;
    
    data.root_canvas = Array<CanvasObject*>::with_size(
        data.allocator, 4
    );
    
    data.touched_focus = Array<CanvasObject*>::with_size(
        data.allocator, 4
    );

	// To avoid any out of range error in handle_input
    data.touched_focus.resize(1);

    data.queue_frees = HashMap<ObjectID, QueueFreeInfo>::with_size(allocator, 4);
}

void SceneManager::shutdown()
{
    if(data.current_scene)
    {
		ObjectCallRef(data.current_scene, exit);
        ObjectAllocator::destroy_object(data.current_scene);
    }

    data.touched_focus.destroy();
    data.root_canvas.destroy();
    data.queue_frees.destroy();

    data.main_viewport.destroy();
}

void SceneManager::change_scene(Object* new_scene)
{
    DebugAssert(new_scene != nullptr, "new scene can't be null");
    DebugAssert(data.change_scene.requested == false, "a change scene was already requested");

    new_scene->set_viewport(&get_main_viewport());

    if (data.current_scene == nullptr)
    {
        data.current_scene = new_scene;
        ObjectCallRef(data.current_scene, enter);
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
        Engine::data.fps = data.fps_counter;
        Log::info(
            "FPS: {}, Avg Frame Time: {}, IntUp: {}, Up: {}, Phy2D: {}, Ren: {}\n"
            "DriRen: {}, DriPresent: {}", 
            data.fps_counter, data.delta_time, data.debug_time.internal_update_time,
            data.debug_time.update_time, data.debug_time.physics_2d_time, 
            data.debug_time.render_time, data.debug_time.driver_render_time,
            data.debug_time.driver_present_time
        );

        data.fps_acum = 0;
        data.time_acum = 0;
    }

    {
        PROFILE_SCOPE(
            data.debug_time.internal_update_time = duration;
        );
        data.current_scene->handle_internal_update(data.delta_time);
    }

    {
        PROFILE_SCOPE(
            data.debug_time.update_time = duration;
        );
        data.current_scene->handle_update(data.delta_time);
    }

    {
        PROFILE_SCOPE(
            data.debug_time.physics_2d_time = duration;
        );
        Physics2D::step(data.delta_time);
    }

    Transform2D camera_transform = Transform2D();
    if (data.current_camera)
    {
        camera_transform = data.current_camera->get_camera_transform(true);
    }
    get_main_viewport().set_scene_transform(camera_transform);

    {
        PROFILE_SCOPE(
            data.debug_time.render_time = duration;
        );
        data.current_scene->handle_render();
    }

    {
        PROFILE_SCOPE(
            data.debug_time.driver_render_time = duration;
        );

        Graphics::render(&get_main_viewport());
        get_main_viewport().reset_commands();
    }

    {
        PROFILE_SCOPE(
            data.debug_time.driver_present_time = duration;
        );
        Graphics::present(&get_main_viewport());
    }

    data.fps_acum++;

    for (auto& it : data.queue_frees)
    {
        it.second.parent->remove_child(it.second.child);
    }

    data.queue_frees.clear();
    _try_clear_root_canvas();
}

void SceneManager::recreate_window()
{
    Graphics::recreate();

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
    data.main_viewport.set_size(new_vp_size);
}

void SceneManager::set_camera_2d(Camera2D* camera)
{
    data.current_camera = camera;
}

void SceneManager::scene_handle_input(const InputEvent& event)
{
    if (data.current_scene == nullptr)
        return;

    if (!data.current_scene->has_mark(Object::MARK_EVENT))
        return;

    switch (event.type)
    {
    case INPUT_EVENT_TOUCH:
    {
        auto& et = event.get<InputEventTouch>();
        InputEventTouch new_event = et;
        new_event = et;

        new_event.position = _screen_make_local_to_canvas(et.position);
        data.touched_focus.resize(et.pointer + 1);
        if (CanvasObject* c = _find_canvas_in_pos(new_event.position))
        {
            data.touched_focus[et.pointer] = c;
            ObjectCallRef(c, gui_event, new_event);
        }
        else
        {
            c = data.touched_focus[et.pointer];
            if (c)
            {
                // point_is_in will be always false
                new_event.pressed = false;
                ObjectCallRef(c, gui_event, new_event);
            }
            data.touched_focus[et.pointer] = nullptr;
        }

        ObjectCallRef(data.current_scene, event, new_event);
    }
    break;
    case INPUT_EVENT_MOUSE_BUTTON:
    {
        auto& et = event.get<InputEventMouseButton>();
        InputEventMouseButton new_event = et;
        new_event = et;

        new_event.position = _screen_make_local_to_canvas(et.position);
        if (CanvasObject* c = _find_canvas_in_pos(new_event.position))
        {
            data.touched_focus[0] = c;
            ObjectCallRef(c, gui_event, new_event);
        }
        else
        {
            c = data.touched_focus[0];
            if (c)
            {
                // point_is_in will be always false
                new_event.pressed = false;
                ObjectCallRef(c, gui_event, new_event);
            }
            data.touched_focus[0] = nullptr;
        }

        ObjectCallRef(data.current_scene, event, new_event);
    }
    break;
    default:
    break;
    }

    data.current_scene->handle_event(event);
}

void SceneManager::_try_clear_root_canvas()
{
    for (usize i = 0; i < data.root_canvas.count; i++)
    {
        CanvasObject* gui_root = data.root_canvas[i];

        if (gui_root == nullptr || !gui_root->has_mark(Object::MARK_QUEUE_FREE))
            continue;

        data.root_canvas.remove(i);
        i--;
    }
}

void SceneManager::_handle_change_scene()
{
    if (data.change_scene.requested == false)
        return;

    data.change_scene.requested = false;
    ObjectCallRef(data.current_scene, exit);
    ObjectAllocator::destroy_object(data.current_scene);

    data.current_scene = data.change_scene.new_scene;
    data.change_scene.new_scene = nullptr;
    ObjectCallRef(data.current_scene, enter);
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

CanvasObject* SceneManager::_find_canvas_in_pos(const Vector2& pos)
{
    for(auto& c : data.root_canvas)
    {
        if(ObjectCallRef(c, is_inside, pos))
        {
            return c;
        }
    }
    
    return nullptr;
}

void SceneManager::_add_root_canvas(CanvasObject* c)
{
    (void)data.root_canvas.add(c);
}

void SceneManager::_queue_free(Object* parent, Object* child)
{
    if(parent)
    {
        (void)data.queue_frees.insert(child->id, QueueFreeInfo(parent, child));
    }
    else
    {
        ObjectAllocator::destroy_object(child);
    }
}


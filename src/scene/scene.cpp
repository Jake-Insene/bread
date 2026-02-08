#include "scene/scene.h"


Scene::SceneClass* Scene::get_class()
{
    static VTable vtable = []()
    {
        VTable tmp = {};
        tmp.construct.bind([](Scene* scene) -> void { ::new (scene) Scene(); });
        tmp.on_create.bind(&Scene::on_create);
        tmp.on_destroy.bind(&Scene::on_destroy);
        tmp.on_enter.bind(&Scene::on_enter);
        tmp.on_exit.bind(&Scene::on_exit);
        tmp.on_internal_update.bind(&Scene::on_internal_update);
        tmp.on_update.bind(&Scene::on_update);
        tmp.on_render.bind(&Scene::on_render);
        tmp.on_internal_update.bind(&Scene::on_internal_update);
        tmp.on_event.bind(&Scene::on_event);
        return tmp;
    }();

    static SceneClass klass
    {
        .super_class = nullptr,
        .scene_name = "Scene",
        .scene_size = sizeof(Scene),
        .vtable = &vtable,
    };
    
    return &klass;
}

Scene* Scene::_allocate_scene(const SceneClass* klass, const CreateInfo& info)
{
    Scene* scene = reinterpret_cast<Scene*>(info.allocator.alloc(klass->scene_size, alignof(Scene)).ptr());
    scene->klass = klass;
    SceneCallRef(scene, on_create, info);
    return scene;
}


// TODO: implement mark
void Scene::mark(MarkName mark_name)
{
    switch (mark_name)
    {
    case MARK_INTERNAL_UPDATE:
    case MARK_UPDATE:
    case MARK_RENDER:
        //SceneManager::_update_object_mark(mark_name, this, true);
        break;
    case MARK_DEALLOCATED:
        //SceneManager::_update_object_mark(mark_name, this, true);
        break;
    default:
        break;
    }

    data.marks.set(mark_name);
}

// TODO: implement unmark
void Scene::unmark(MarkName mark_name)
{
    switch (mark_name)
    {
    case MARK_INTERNAL_UPDATE:
    case MARK_UPDATE:
    case MARK_RENDER:
        //SceneManager::_update_object_mark(mark_name, this, false);
        break;
    default:
        break;
    }

    data.marks.unset(mark_name);
}

// TODO: implement queue_free
void Scene::queue_free()
{
    if (has_mark(MARK_QUEUE_FREE))
        return;

    mark(MARK_QUEUE_FREE);

    //SceneManager::_queue_free(get_parent(), this);
}

void Scene::set_group(GroupName group_name, bool value)
{
    if (value)
        data.bit_groups.set(group_name);
    else
        data.bit_groups.unset(group_name);
}

bool Scene::has_group(GroupName group_name) const
{
    return data.bit_groups.is_set(group_name);
}

void Scene::set_viewport(Viewport* new_viewport)
{
    if (data.viewport == new_viewport)
        return;

    data.viewport = new_viewport;
}

void Scene::on_create(const CreateInfo& info)
{
    allocator = info.allocator;
    
    data.name = String::with_allocator(allocator);
    data.bit_groups = {};
    data.marks = {};
    data.viewport = nullptr;
}

void Scene::on_destroy() {}

void Scene::on_enter() {}

void Scene::on_exit() {}

void Scene::on_update(f32) {}

void Scene::on_internal_update(f32) {}

void Scene::on_render(f32) {}

void Scene::on_event(const InputEvent&) {}

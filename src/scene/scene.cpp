#include "scene/scene.h"


Scene Scene::create(const mem::Allocator& allocator)
{
    Scene new_scene = {};
    new_scene.data.allocator = allocator;
    new_scene.data.objects = Array<Ptr<GameObject>>::with_allocator(allocator);

    return new_scene;
}

void Scene::destroy()
{
    for(Ptr<GameObject> obj : data.objects.iter())
    {
        obj.destroy(data.allocator);
    }

    data.objects.destroy();
}

Ptr<GameObject> Scene::add_object()
{
    return data.objects.add(Ptr<GameObject>::create(data.allocator));
}

void Scene::add_allocated_object(Ptr<GameObject> new_object)
{
    (void)data.objects.add(new_object);
}

#pragma once
#include "collections/array.h"
#include "collections/event.h"
#include "collections/ptr.h"
#include "collections/string_view.h"



struct GameObject
{};


#define SCENE_BODY(TYPE) \
    static SceneClass* get_class()\
    {\
        static bool is_initialized = false;\
        static SceneClass klass = {};\
        static TYPE::VTable vtable = {};\
        if(!is_initialized)\
        {\
            vtable = []()\
            {\
                TYPE::VTable tmp = {};\
                tmp.construct.bind([](Scene* scene) -> void { ::new (reinterpret_cast<TYPE*>(scene)) TYPE(); });\
                tmp.on_create.bind(&TYPE::on_create);\
                tmp.on_destroy.bind(&TYPE::on_destroy);\
                return tmp;\
            }();\
            klass.scene_name = #TYPE;\
            klass.scene_size = sizeof(TYPE);\
            klass.vtable = &vtable;\
            is_initialized = true;\
        }\
        return &klass;\
    }\

    
struct Scene
{
    struct VTable
    {
        Event<void(*)(Scene*)> construct;
        Event<void(Scene::*)(), false> on_create;
        Event<void(Scene::*)(), false> on_destroy;
    };

    struct SceneClass
    {
        StringView scene_name;
        usize scene_size;
        VTable* vtable;
    };

    struct InternalData
    {
        mem::Allocator allocator;
        Array<Ptr<GameObject>> objects;
    } data;

    static Scene create(const mem::Allocator& allocator);

    void destroy();

    Ptr<GameObject> add_object();
    void add_allocated_object(Ptr<GameObject> new_object);

    void on_create();
    void on_destroy();
};


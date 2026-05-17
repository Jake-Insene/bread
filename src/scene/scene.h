#pragma once
#include "collections/event.h"
#include "collections/bits.h"
#include "collections/string.h"
#include "collections/string_view.h"
#include "input/input.h"
#include "mem/allocator.h"



struct Viewport;


#define SCENE_FUNCV(CLASS, BASE, NAME)\
    void(Scene::*get_##NAME()) ()\
    {\
        return reinterpret_cast<void(Scene::*)()>(&CLASS::NAME);\
    }\
    void NAME##v()\
    {\
        BASE::NAME##v();\
        if(CLASS::get_##NAME() != BASE::get_##NAME())\
        {\
            CLASS::NAME();\
        }\
    }
    
#define SCENE_RFUNCV(CLASS, BASE, NAME)\
    void(Scene::*get_##NAME()) ()\
    {\
        return reinterpret_cast<void(Scene::*)()>(&CLASS::NAME);\
    }\
    void NAME##v()\
    {\
        if(CLASS::get_##NAME() != BASE::get_##NAME())\
        {\
            CLASS::NAME();\
        }\
        BASE::NAME##v();\
    }

#define SCENE_FUNCV_ARG1(CLASS, BASE, NAME, ARG0)\
    void(Scene::*get_##NAME()) (ARG0)\
    {\
        return reinterpret_cast<void(Scene::*)(ARG0)>(&CLASS::NAME);\
    }\
    void NAME##v(ARG0 _0)\
    {\
        BASE::NAME##v(_0);\
        if(CLASS::get_##NAME() != BASE::get_##NAME())\
        {\
            CLASS::NAME(_0);\
        }\
    }


#define SCENE_BODY(CLASS, BASE) \
    static SceneClass* get_class()\
    {\
        static bool is_initialized = false;\
        static SceneClass klass = {};\
        static CLASS::VTable vtable = {};\
        if(!is_initialized)\
        {\
            vtable = []()\
            {\
                CLASS::VTable tmp = {};\
                tmp.construct.bind([](Scene* scene) -> void { ConstructObject(*reinterpret_cast<CLASS*>(scene)); });\
                tmp.on_create.bind(&CLASS::on_createv);\
                tmp.on_destroy.bind(&CLASS::on_destroyv);\
                tmp.on_enter.bind(&CLASS::on_enterv);\
                tmp.on_exit.bind(&CLASS::on_exitv);\
                tmp.on_update.bind(&CLASS::on_update);\
                tmp.on_internal_update.bind(&CLASS::on_internal_updatev);\
                tmp.on_render.bind(&CLASS::on_render);\
                tmp.on_event.bind(&CLASS::on_eventv);\
                return tmp;\
            }();\
            klass.super_class = BASE::get_class();\
            klass.scene_name = #CLASS;\
            klass.scene_size = sizeof(CLASS);\
            klass.vtable = &vtable;\
            is_initialized = true;\
        }\
        return &klass;\
    }\
    void(Scene::*get_on_create()) (const CreateInfo&)\
    {\
        return reinterpret_cast<void(Scene::*)(const CreateInfo&)>(&CLASS::on_create);\
    }\
    void on_createv(const CreateInfo& info)\
    {\
        BASE::on_createv(info);\
        for(const MarkName& m : CLASS::SceneClassMarks) { mark(m); }\
        if(CLASS::get_on_create() != BASE::get_on_create())\
        {\
            CLASS::on_create(info);\
        }\
    }\
    SCENE_RFUNCV(CLASS, BASE, on_destroy)\
    SCENE_FUNCV(CLASS, BASE, on_enter)\
    SCENE_FUNCV(CLASS, BASE, on_exit)\
    SCENE_FUNCV_ARG1(CLASS, BASE, on_internal_update, f32)\
    SCENE_FUNCV_ARG1(CLASS, BASE, on_event, const InputEvent&)\

    
#define SceneCall(name, ...) \
    static_cast<RemoveConstPointer<decltype(this)>::VTable&>(*klass->vtable).name.call(this __VA_OPT__(,) __VA_ARGS__)

#define SceneCallRef(ref, name, ...) \
    static_cast<RemoveConstPointer<decltype((ref))>::VTable&>(*(ref)->klass->vtable).name.call(ref __VA_OPT__(,) __VA_ARGS__)




struct Scene
{
    using GroupName = u64;
    using MarkName = u64;

    enum
    {
        /*
        * Default mark, only serves as a place holder.
        */
        MARK_DEFAULT,

        /*
        * The scene has internal behaviour that needs to be preserved.
        */
        MARK_INTERNAL_UPDATE,

        /*
        * The scene has unique behaviour, can overrided by its derived classes.
        */
        MARK_UPDATE,

        /*
        * The scene has rendering behaviour, can overrided by its derived classes.
        */
        MARK_RENDER,

        /*
        * Input devices can interact with the scene.
        */
        MARK_EVENT,

        /*
        * The scene is the main scene.
        */
        MARK_IN_SCENE,

        /*
        * The scene was marked to be deleted at the end of the frame.
        */
        MARK_QUEUE_FREE,

        /*
        * The scene is deallocated.
        */
        MARK_DEALLOCATED,

        MARK_COUNT,
    };

    struct CreateInfo
    {
        mem::Allocator* allocator;
    };

    struct VTable
    {
        Event<void(*)(Scene*)> construct;
        Event<void(Scene::*)(const CreateInfo&), false> on_create;
        Event<void(Scene::*)(), false> on_destroy;
        Event<void(Scene::*)(), false> on_enter;
        Event<void(Scene::*)(), false> on_exit;
        Event<void(Scene::*)(f32), false> on_update;
        Event<void(Scene::*)(f32), false> on_internal_update;
        Event<void(Scene::*)(f32), false> on_render;
        Event<void(Scene::*)(const InputEvent&), false> on_event;
    };

    struct SceneClass
    {
        SceneClass* super_class;
        StringView scene_name;
        usize scene_size;
        VTable* vtable;
    };

    static constexpr MarkName SceneClassMarks[] = { MARK_DEFAULT };

    static SceneClass* get_class();

    static Scene* _allocate_scene(const SceneClass* klass, const CreateInfo& info);
    
    template<typename T>
    requires(IsBaseOf<Scene, T>)
    [[nodiscard]] static T* create(mem::Allocator* allocator)
    {
        CreateInfo info = 
        {
            .allocator = allocator,
        };

        return reinterpret_cast<T*>(_allocate_scene(T::get_class(), info));
    }

    // Object callbacks
#define SCENE_FDEFAULT(NAME)\
    void(Scene::*get_##NAME()) ()\
    {\
        return reinterpret_cast<void(Scene::*)()>(&Scene::NAME);\
    }\
    void NAME##v()\
    {\
        NAME();\
    }\

#define SCENE_FDEFAULT_ARG1(NAME, ARG0)\
    void(Scene::*get_##NAME()) (ARG0)\
    {\
        return reinterpret_cast<void(Scene::*)(ARG0)>(&Scene::NAME);\
    }\
    void NAME##v(ARG0 _0)\
    {\
        NAME(_0);\
    }\

    // Only for recursive functions.
    // update and render should not be recursive.
    SCENE_FDEFAULT_ARG1(on_create, const CreateInfo&);
    SCENE_FDEFAULT(on_destroy);
    SCENE_FDEFAULT(on_enter);
    SCENE_FDEFAULT(on_exit);
    SCENE_FDEFAULT_ARG1(on_internal_update, f32);
    SCENE_FDEFAULT_ARG1(on_event, const InputEvent&);

#undef SCENE_DEFAULT
#undef SCENE_FDEFAULT_ARG1

    /*
    * Scene data
    */
    const SceneClass* klass;

    mem::Allocator* allocator;
    
    struct InternalData
    {
        String name; // necessary?

        BitMask<MARK_COUNT> marks;
        BitMask<64> bit_groups;

        Viewport* viewport;
    } data;

    /*
	* @param mark The mark to check.
    * @return True if the mark is enable, false otherwise.
    */
    [[nodiscard]] bool has_mark(MarkName mark_name) const { return data.marks.is_set(mark_name); }

    /*
    * @param mark The mark to enable.
    */
    void mark(MarkName mark_name);
   
    /*
    * @param mark The mark to disable.
    */
    void unmark(MarkName mark_name);

    /*
    * @param group_bit The group bit to set.
    * @param value The group bit value.
    */
    void set_group(GroupName group_name, bool value);

    /*
	* @param group_bit The group bit to check.
    * @return The value of the group bit.
    */
    [[nodiscard]] bool has_group(GroupName group_name) const;

    /*
    * Perform a safe free of the scene, deleting it at the end of the frame.
    */
    void queue_free();

    /*
    * @return The viewport where the object is being rendered.
    */
    [[nodiscard]] Viewport* get_viewport() const { return data.viewport; }

    /*
    * Do not use directly.
    */
    void set_viewport(Viewport* new_viewport);

    void on_create(const CreateInfo& info);
    
    void on_destroy();

    void on_enter();

    void on_exit();

    void on_update(f32 dt);

    void on_render(f32 dt);

    void on_internal_update(f32 dt);

    void on_event(const InputEvent& event);
};



template<typename T>
concept IsScene = IsBaseOf<Scene, T>;

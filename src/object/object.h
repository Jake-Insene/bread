#pragma once
#include "collections/array.h"
#include "collections/bits.h"
#include "collections/event.h"
#include "collections/hash_map.h"
#include "collections/string.h"
#include "object/object_id.h"


#define OBJECT_FUNCV(klass, base, name)\
    void(Object::*get_##name()) ()\
    {\
        return (void(Object::*)())&klass::name;\
    }\
    void name##v()\
    {\
        base::name##v();\
        if(klass::get_##name() != base::get_##name())\
        {\
            klass::name();\
        }\
    }
    
#define OBJECT_RFUNCV(klass, base, name)\
    void(Object::*get_##name()) ()\
    {\
        return (void(Object::*)())&klass::name;\
    }\
    void name##v()\
    {\
        if(klass::get_##name() != base::get_##name())\
        {\
            klass::name();\
        }\
        base::name##v();\
    }

#define OBJECT_FUNCV_ARG1(klass, base, name, arg0)\
    void(Object::*get_##name()) (arg0)\
    {\
        return (void(Object::*)(arg0))&klass::name;\
    }\
    void name##v(arg0 _0)\
    {\
        base::name##v(_0);\
        if(klass::get_##name() != base::get_##name())\
        {\
            klass::name(_0);\
        }\
    }

#define OBJECT(name, base, ...)\
    static void* _get_bind_vtable()\
    {\
        return reinterpret_cast<void*>(&name::_bind_vtable);\
    }\
    static bool _try_bind_vtable(name::VTable& vtable)\
    {\
        base::_try_bind_vtable(vtable);\
        if(name::_get_bind_vtable() != base::_get_bind_vtable())\
        {\
            name::_bind_vtable(static_cast<name::VTable&>(vtable));\
        }\
        return true;\
    }\
    static Class* get_class()\
    {\
        static name::VTable vtable = []()\
        {\
            name::VTable tmp = {};\
            tmp.construct.bind([](Object* obj) -> void { ::new ((name*)obj) name(); });\
            tmp.init.bind(&name::initv);\
            tmp.deinit.bind(&name::deinitv);\
            tmp.enter.bind(&name::enterv);\
            tmp.internal_update.bind(&name::internal_updatev);\
            tmp.update.bind(&name::update);\
            tmp.render.bind(&name::render);\
            tmp.exit.bind(&name::exitv);\
            tmp.event.bind(&name::eventv);\
            return tmp;\
        }();\
        static bool unused = name::_try_bind_vtable(vtable);\
        (void)unused;\
        static Class klass\
        {\
            .class_name = #name,\
            .class_size = sizeof(name),\
            .vtable = vtable,\
        };\
        return &klass;\
    }\
    \
    OBJECT_FUNCV_ARG1(name, base, init, const CreateInfo&)\
    OBJECT_RFUNCV(name, base, deinit)\
    OBJECT_FUNCV(name, base, enter)\
    OBJECT_FUNCV_ARG1(name, base, internal_update, f32)\
    OBJECT_FUNCV(name, base, exit)\
    OBJECT_FUNCV_ARG1(name, base, event, const InputEvent&)\
    
    
// Don't use VTableCall because it reference the member vtable that
// is not in an object.
#define ObjectCall(name, ...) \
    static_cast<RemoveConstPointer<decltype(this)>::VTable&>(klass->vtable).name.call(this, __VA_ARGS__)

#define ObjectCallRef(ref, name, ...) \
    static_cast<RemoveConstPointer<decltype(ref)>::VTable&>(ref->klass->vtable).name.call(ref, __VA_ARGS__)


#define DefineVTable(base) struct VTable : base::VTable

struct Object;
struct InputEvent;

/*
* The minimum entity that can be placed in a scene, can safely instanced in a scene.
* A scene is just a object instanced without a parent.
*/
struct Object
{
    struct CreateInfo
    {
        mem::Allocator allocator;
    };

    struct VTable
    {
        Event<void(*)(Object*)> construct;

        Event<void(Object::*)(const CreateInfo&), false> init;
        Event<void(Object::*)(), false> deinit;

        Event<void(Object::*)(), false> enter;
        Event<void(Object::*)(f32), false> internal_update;
        Event<void(Object::*)(f32), false> update;
        Event<void(Object::*)(), false> render;
        Event<void(Object::*)(), false> exit;

        Event<void(Object::*)(const InputEvent&), false> event;
    };

    struct Class
    {
        StringView class_name;
        usize class_size;
        VTable& vtable;
    };

    static void* _get_bind_vtable() { return reinterpret_cast<void*>(&Object::_bind_vtable); }

    static void _try_bind_vtable(VTable& vtable)
    {
        return _bind_vtable(vtable);
    }

    static void _bind_vtable(VTable& vtable);

    // Non-static fields

    ObjectID id;
    const Class* klass{};
    mem::Allocator allocator{};

    enum
    {
        MARK_INTERNAL_UPDATE,
        MARK_UPDATE,
        MARK_RENDER,
        MARK_EVENT,
        MARK_IN_SCENE,

        MARK_2D,
        MARK_CANVAS,

        MARK_QUEUE_FREE,

        MARK_COUNT,
    };

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        String name{}; // necessary?
        Object* parent = nullptr;
        Array<Object*> childs;

        BitField<MARK_COUNT> marks{};
        BitField<64> bit_groups;
    } data;

    // Internal, you should not use them
    void handle_internal_update(f32 dt);
    void handle_update(f32 dt);
    void handle_render();
    void handle_event(const InputEvent& e);

    // Query info
    void set_mark(u64 mark, bool value)
    { 
        if (value)
            data.marks.set(mark);
        else
            data.marks.unset(mark);
    }

    [[nodiscard]] bool has_mark(u64 mark) const { return data.marks.is_set(mark); }

    void mark(u64 mark) { data.marks.set(mark); }
    void unmark(u64 mark) { data.marks.unset(mark); }

    void set_group(u64 group_bit, bool value);
    [[nodiscard]] bool has_group(u64 group_bit) const { return data.bit_groups.is_set(group_bit); }

    // Object std functions

    template<typename T>
    [[nodiscard]] T* cast() const { return (T*)this; }

    // Can be null on root scene
    [[nodiscard]] Object* get_parent() const { return data.parent; }

    void add_child(Object* obj);
    void remove_child(Object* obj);
    [[nodiscard]] usize get_child_count() const { return data.childs.count; }
    Object* get_child(usize index) { return data.childs[index]; }

    void queue_free();
    
    // Object callbacks
#define OBJECT_FDEFAULT(name)\
    void(Object::*get_##name()) ()\
    {\
        return (void(Object::*)())&Object::name;\
    }\
    void name##v()\
    {\
        name();\
    }\
    
#define OBJECT_FDEFAULT_ARG1(name, arg0)\
    void(Object::*get_##name()) (arg0)\
    {\
        return (void(Object::*)(arg0))&Object::name;\
    }\
    void name##v(arg0 _0)\
    {\
        name(_0);\
    }\
    
    // Only for recursive functions
    // update and render should not be recursive
    OBJECT_FDEFAULT_ARG1(init, const CreateInfo&);
    OBJECT_FDEFAULT(deinit);
    OBJECT_FDEFAULT(enter);
    OBJECT_FDEFAULT_ARG1(internal_update, f32);
    OBJECT_FDEFAULT(exit);
    OBJECT_FDEFAULT_ARG1(event, const InputEvent&);
    
#undef OBJECT_DEFAULT
#undef OBJECT_FDEFAULT_ARG1
    
    /*
    * Called after the object is allocated.
    * 
    * @param info Contains information about the object creation.
    */
    void init(const CreateInfo& info) Function(FunctionPropagate);

    /*
    * Called before the object is deallocated.
    */
    void deinit() Function(FunctionPropagate);

    /*
    * Called after the object is instanced in the main scene.
    */
    void enter() Function(FunctionPropagate);

    /*
    * Called every frame like update(f64).
    * 
    * Used to create inherit behavior.
    * 
    * Mark: MARK_INTERNAL_UPDATE
    * @param dt The elapsed time since the last frame.
    */
    void internal_update(f32) Function(FunctionPropagate) {}

    /*
    * Called every frame. Used to create object behavior.
    * 
    * Mark: MARK_UPDATE
    * @param dt The elapsed time since the last frame
    */
    void update(f32) {}

    /*
    * Called every frame to request draw commands.
    * 
    * Mark: MARK_RENDER
    */
    void render() {}

    /*
    * Called after the object exit from the main scene.
    */
    void exit() Function(FunctionPropagate);
    
    /*
    * Called when the application receives input from a input device.
    * 
    * See InputEventType.
    * 
    * @param event Contains information about the input that triggers the call.
    * 
    * @Function(PropagateToChildren)
    */
    void event(const InputEvent& event) Function(FunctionPropagate);
};

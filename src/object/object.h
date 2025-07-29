#pragma once
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

#define OBJECT(name, base)\
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
    OBJECT_FUNCV_ARG1(name, base, internal_update, f64)\
    OBJECT_FUNCV(name, base, exit)\
    OBJECT_FUNCV_ARG1(name, base, event, const InputEvent&)\
    
    
// Dont use VTableCall because it reference the member vtable that
// is not in an object.
#define ObjectCall(name, ...) \
    static_cast<RemoveConstPointer<decltype(this)>::VTable&>(klass->vtable).name.call(this __VA_OPT__(,) __VA_ARGS__)

#define ObjectCallRef(ref, name, ...) \
    static_cast<RemoveConstPointer<decltype(ref)>::VTable&>(ref->klass->vtable).name.call(ref __VA_OPT__(,) __VA_ARGS__)


#define DefineVTable(base) struct VTable : base::VTable


struct InputEvent;

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
        Event<void(Object::*)(f64), false> internal_update;
        Event<void(Object::*)(f64), false> update;
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
    
    ObjectID id;
    const Class* klass{};
    mem::Allocator allocator{};
    
    enum
    {
        MARK_UPDATE,
        MARK_RENDER,
        MARK_HANDLE_EVENT,
        MARK_IN_SCENE,
        MARK_INTERNAL_UPDATE,

        MARK_2D,
        MARK_CANVAS,

        MARK_QUEUE_FREE,

        MARK_COUNT,
    };

    enum
    {
        MARK_DISABLE = 0,
        MARK_ENABLE = 1,
    };
    
    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        String name{}; // necessary?
        Object* parent = nullptr;
        HashMap<ObjectID, Object*> childs{};

        BitField<MARK_COUNT> marks{};
        BitField<64> bit_groups;
    } data;
    
    // Internal, you should not use them
    void handle_internal_update(f64 dt);
    void handle_update(f64 dt);
    void handle_render();

    // Query info
    [[nodiscard]] bool has_mark(u64 mark) const { return data.marks.is_set(mark); }
    void set_mark(u64 mark, bool value) { data.marks.set(mark, value); }
    void mark(u64 mark) { data.marks.set(mark, MARK_ENABLE); }
    void unmark(u64 mark) { data.marks.unset(mark); }

    [[nodiscard]] bool has_group(u64 group_bit) const { return data.bit_groups.is_set(group_bit); }
    void set_group(u64 group_bit, bool value);
    
    // Object std functions

    template<typename T>
    [[nodiscard]] T* cast() const { return (T*)this; }
    
    // Can be null on root scene
    [[nodiscard]] Object* get_parent() const { return data.parent; }

    void add_child(Object* obj);
    void remove_child(Object* obj);
    [[nodiscard]] usize get_child_count() const { return data.childs.count; }

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
    OBJECT_FDEFAULT_ARG1(internal_update, f64);
    OBJECT_FDEFAULT(exit);
    OBJECT_FDEFAULT_ARG1(event, const InputEvent&);
    
#undef OBJECT_DEFAULT
#undef OBJECT_FDEFAULT_ARG1
    
    void init(const CreateInfo& info);
    void deinit();

    void enter();
    void internal_update(f64) {}
    void update(f64) {}
    void render() {}
    void exit();
    
    void event(const InputEvent& event);
};

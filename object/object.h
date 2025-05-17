#pragma once
#include "core/header.h"
#include "input/input.h"
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
    static void(*get_bind_vtable()) (Object::VTable&)\
    {\
        return reinterpret_cast<void(*)(Object::VTable&)>(&name::_bind_vtable);\
    }\
    static bool try_bind_vtable(name::VTable& vtable)\
    {\
        base::try_bind_vtable(vtable);\
        if(name::get_bind_vtable() != base::get_bind_vtable())\
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
            tmp.construct = [](Object* obj) -> void { ::new ((name*)obj) name(); };\
            tmp.init = VTCastGet(Object::VTable::init, name::initv);\
            tmp.deinit = VTCastGet(Object::VTable::deinit, name::deinitv);\
            tmp.enter = VTCastGet(Object::VTable::enter, name::enterv);\
            tmp.internal_update = VTCastGet(Object::VTable::internal_update, name::internal_updatev);\
            tmp.update = VTCastGet(Object::VTable::update, name::update);\
            tmp.render = VTCastGet(Object::VTable::render, name::render);\
            tmp.exit = VTCastGet(Object::VTable::exit, name::exitv);\
            tmp.event = VTCastGet(Object::VTable::event, name::eventv);\
            return tmp;\
        }();\
        static bool unused = name::try_bind_vtable(vtable);\
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
#define ObjectCallVTable(ref, vtable) ((RemoveConstPointer<decltype(ref)>::VTable&)vtable)
#define ObjectCall(name, ...) (*this.*ObjectCallVTable(this, this->klass->vtable).name)(__VA_ARGS__)

#define ObjectCallRef(ref, name, ...) (*ref.*ObjectCallVTable(ref, ref->klass->vtable).name)(__VA_ARGS__)


#define DefineVTable(base) struct VTable : base::VTable


struct Object
{
    struct CreateInfo
    {
        mem::Allocator allocator;
    };
    
    struct VTable
    {
        void(*construct)(Object*);

        void(Object::*init)(const CreateInfo&);
        void(Object::*deinit)();
        
        void(Object::*enter)();
        void(Object::*internal_update)(f64);
        void(Object::*update)(f64);
        void(Object::*render)();
        void(Object::*exit)();
        
        void(Object::*event)(const InputEvent&);
    };
    
    struct Class
    {
        StringView class_name;
        usize class_size;
        VTable& vtable;
    };
    
    static void(*get_bind_vtable()) (VTable&) { return &Object::_bind_vtable; }
    
    static void try_bind_vtable(VTable& vtable)
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
        MARK_CONTROL,

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
    
    // Object std functions
    
    // Can be null on root scene
    [[nodiscard]] Object* get_parent() const { return data.parent; }

    void add_child(Object* obj);
    void remove_child(Object* obj);

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

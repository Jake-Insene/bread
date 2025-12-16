#pragma once
#include "collections/array.h"
#include "collections/bits.h"
#include "collections/event.h"
#include "collections/hash_map.h"
#include "collections/string.h"
#include "collections/string_view.h"
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
    static void _try_bind_vtable(name::VTable& vtable)\
    {\
        base::_try_bind_vtable(vtable);\
        if(name::_get_bind_vtable() != base::_get_bind_vtable())\
        {\
            name::_bind_vtable(static_cast<name::VTable&>(vtable));\
        }\
    }\
    static Class* get_class()\
    {\
        static bool is_initialized = false;\
        static Class klass = {};\
        static name::VTable vtable = {};\
        if(!is_initialized)\
        {\
            vtable = []()\
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
                name::_try_bind_vtable(tmp);\
                return tmp;\
            }();\
            klass.super_class = base::get_class();\
            klass.class_name = #name;\
            klass.class_size = sizeof(name);\
            klass.vtable = &vtable;\
            is_initialized = true;\
        }\
        return &klass;\
    }\
    \
    void(Object::*get_init()) (const CreateInfo&)\
    {\
        return (void(Object::*)(const CreateInfo&))&name::init;\
    }\
    void initv(const CreateInfo& _0)\
    {\
        base::initv(_0);\
        for(const MarkName& m : name::ClassMarks) { mark(m); }\
        if(name::get_init() != base::get_init())\
        {\
            name::init(_0);\
        }\
    }\
    OBJECT_RFUNCV(name, base, deinit)\
    OBJECT_FUNCV(name, base, enter)\
    OBJECT_FUNCV(name, base, exit)\
    OBJECT_FUNCV_ARG1(name, base, internal_update, f32)\
    OBJECT_FUNCV_ARG1(name, base, event, const InputEvent&)\
    

#define MARKS(...) static constexpr MarkName ClassMarks[] = {__VA_ARGS__};

    
// Don't use VTableCall because it reference the member vtable that
// is not in an object.
#define ObjectCall(name, ...) \
    static_cast<RemoveConstPointer<decltype(this)>::VTable&>(*klass->vtable).name.call(this __VA_OPT__(,) __VA_ARGS__)

#define ObjectCallRef(ref, name, ...) \
    static_cast<RemoveConstPointer<decltype(ref)>::VTable&>(*ref->klass->vtable).name.call(ref __VA_OPT__(,) __VA_ARGS__)


#define DefineVTable(base) struct VTable : base::VTable

#define BindVTable(vtable, func_name, address) vtable.func_name.bind(address)

struct Object;
struct InputEvent;
struct Viewport;
using GroupName = u64;
using MarkName = u64;

/*
* The minimum entity that can be placed in a scene, can safely instanced in a scene.
* A scene is just a object instanced without a parent.
*/
struct Object
{
    enum
    {
        /*
        * Default mark, only serves as a place holder.
        */
        MARK_DEFAULT,

        /*
        * The object has internal behaviour that needs to be preserved.
        */
        MARK_INTERNAL_UPDATE,

        /*
        * The object has unique behaviour, can overrided by its derived classes.
        */
        MARK_UPDATE,

        /*
        * The object has rendering behaviour, can overrided by its derived classes.
        */
        MARK_RENDER,

        /*
        * Input devices can interact with the object.
        */
        MARK_EVENT,

        /*
        * The object is in the main scene.
        */
        MARK_IN_SCENE,

        /*
        * The object is a 2D world element.
        */
        MARK_2D,

        /*
        * The object is a 2D world element.
        */
        MARK_CANVAS,

        /*
        * The object was marked to be deleted at the end of the frame.
        */
        MARK_QUEUE_FREE,

        /*
        * The object is deallocated.
        */
        MARK_DEALLOCATED,

        MARK_COUNT,
    };

    /*
    * As soon as you can see struct/clases in the engine are always public,
    * this is a design pattern, to expose public read/write data you can
    * create member function or let the user acces directly to them, for private
    * data you should use a 'data' field, this way you separate public from private data members
    * in a visual way.
    */
    struct InternalData
    {
        String name{}; // necessary?
        Object* parent = nullptr;
        Array<Object*> childs;

        BitMask<MARK_COUNT> marks{};
        BitMask<64> bit_groups{};

        Viewport* viewport = nullptr;
    } data;

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
        Event<void(Object::*)(), false> exit;
        Event<void(Object::*)(f32), false> internal_update;
        Event<void(Object::*)(f32), false> update;
        Event<void(Object::*)(), false> render;

        Event<void(Object::*)(const InputEvent&), false> event;
    };

    struct Class
    {
        Class* super_class;
        StringView class_name;
        usize class_size;
        VTable* vtable;
    };

    static constexpr MarkName ClassMarks[] = { MARK_DEFAULT };

    static void* _get_bind_vtable() { return reinterpret_cast<void*>(&Object::_bind_vtable); }

    static void _try_bind_vtable(VTable&) {}
    static void _bind_vtable(VTable&);

    static Class* get_class()
    {
        static VTable vtable = []()
        {
            VTable tmp = {};
            tmp.construct.bind([](Object* obj) -> void { ::new (obj) Object(); });
            tmp.init.bind(&Object::initv);
            tmp.deinit.bind(&Object::deinitv);
            tmp.enter.bind(&Object::enterv);
            tmp.internal_update.bind(&Object::internal_updatev);
            tmp.update.bind(&Object::update);
            tmp.render.bind(&Object::render);
            tmp.exit.bind(&Object::exitv);
            tmp.event.bind(&Object::eventv);
            return tmp;
        }();

        static Class klass
        {
            .super_class = nullptr,
            .class_name = "Object",
            .class_size = sizeof(Object),
            .vtable = &vtable,
        };

        return &klass;
    }

    static Object* _get_by_id(ObjectID id) Function(FunctionInternal);

    /*
    * @param Object Object to check.
    * 
    * @return True if the object is a subclass of T, false otherwise.
    */
    template<typename T>
    [[nodiscard]] static bool is_class_of(Object* object)
    {
        if (object == nullptr)
            return false;

        const Class* klass = object->klass;
        while (klass)
        {
            if (klass == T::get_class())
                return true;

            klass = klass->super_class;
        }

        return false;
    }

    /*
    * Not safe, direct cast of the object.
    * 
    * @return The object casted to T, if the object class is not T returns nullptr.
    */
    template<typename T>
    [[nodiscard]] static T* cast(Object* object)
    {
        return is_class_of<T>(object) ? reinterpret_cast<T*>(object) : nullptr;
    }

    [[nodiscard]] static Object* create_from_class(const Class* object_klass);

    /*
    * Create an object of the given type.
    * It doesn't put it in the scene tree, You must explicitly call add_child.
    * 
    * @return The allocated object.
    */ 
    template<typename T>
        requires(IsBaseOf<Object, T>)
    [[nodiscard]] static T* create()
    {
        return reinterpret_cast<T*>(create_from_class(T::get_class()));
    }

    /*
    * Get the object referenced by the id.
    * 
    * @return The object that owns the id.
    */
    template<typename T>
    [[nodiscard]] static T* get_by_id(ObjectID id)
    {
        return reinterpret_cast<T*>(_get_by_id(id));
    }

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

    // Only for recursive functions.
    // update and render should not be recursive.
    OBJECT_FDEFAULT_ARG1(init, const CreateInfo&);
    OBJECT_FDEFAULT(deinit);
    OBJECT_FDEFAULT(enter);
    OBJECT_FDEFAULT(exit);
    OBJECT_FDEFAULT_ARG1(internal_update, f32);
    OBJECT_FDEFAULT_ARG1(event, const InputEvent&);

#undef OBJECT_DEFAULT
#undef OBJECT_FDEFAULT_ARG1

    // Non-static fields

    ObjectID id{};
    const Class* klass{};
    /*
    * The object memory allocator, Use it to allocate memory for the object.
    */
    mem::Allocator allocator{};

    void handle_event(const InputEvent& event) Function(FunctionInternal);

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
    * @return The viewport where the object is being rendered.
    */
    [[nodiscard]] Viewport* get_viewport() const { return data.viewport; }

    /*
    * Do not use directly.
    */
    void set_viewport(Viewport* new_vp);

    /*
	* @return The parent of the object, if it has no parent return nullptr.
    */
    [[nodiscard]] Object* get_parent() const { return data.parent; }

    /*
    * Add the child to the object, if the object is in the main scene
    * it will call enter() on the child and propagate it to its childs.
    *
    * @param request_child The child to add.
    */
    void add_child(Object* request_child);

    /*
    * Finds and removes the given object child,
    * only check the childs of the object not its sub childs.
    * 
    * @param child The child to remove.
    */
    void remove_child(Object* child);

    /*
    * @return The number of childs of the object.
    */
    [[nodiscard]] usize get_child_count() const { return data.childs.count; }

    /*
    * Get the child at the given index.
    * 
    * @param index The index of the child to get.
    */
    Object* get_child(usize index) { return data.childs.get(index); }

    /*
    * Perform a safe free of the object, deleting it at the end of the frame.
    * The childs of the object are also deleted.
    */
    void queue_free();
    
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
    void enter() RequireMark(MARK_IN_SCENE) Function(FunctionPropagate);

    /*
    * Called after the object exit from the main scene.
    */
    void exit() RequireMark(MARK_IN_SCENE) Function(FunctionPropagate);

    /*
    * Called every frame like update(f32).
    *
    * Used to create inherit behaviour.
    *
    * Mark: MARK_INTERNAL_UPDATE
    * @param dt The elapsed time since the last frame.
    */
    void internal_update(f32) RequireMark(MARK_INTERNAL_UPDATE) Function(FunctionPropagate)
    {}

    /*
    * Called every frame. Used to create object behaviour.
    * 
    * Mark: MARK_UPDATE
    */
    void update(f32) RequireMark(MARK_UPDATE) {}

    /*
    * Called every frame to request draw commands.
    * 
    * Mark: MARK_RENDER
    */
    void render() RequireMark(MARK_RENDER) {}

    /*
    * Called when the application receives input from a input device.
    * 
    * See InputEventType.
    * 
    * @param event Contains information about the input that triggers the call.
    */
    void event(const InputEvent& event) RequireMark(MARK_EVENT) Function(FunctionPropagate);
};

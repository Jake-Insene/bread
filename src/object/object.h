#pragma once
#include "collections/array.h"
#include "collections/bits.h"
#include "collections/event.h"
#include "collections/string.h"
#include "collections/string_view.h"
#include "object/object_id.h"


struct Object;
struct InputEvent;
using GroupName = u64;
using MarkName = u64;

/*
* The minimum entity that can be placed in a scene, can safely instanced in a scene.
* A scene is just a object instanced without a parent.
*/
struct Object
{
    /*
    * As soon as you can see struct/clases in the engine are always public,
    * this is a design pattern, to expose public read/write data you can
    * create member function or let the user acces directly to them, for private
    * data you should use a 'data' field, this way you separate public from private data members
    * in a visual way.
    */
    struct InternalData
    {
        String name; // necessary?
        BitMask<64> bit_groups;
    };

    struct CreateInfo
    {
        mem::Allocator* allocator;
    };

    struct VTable
    {
        Event<void(*)(Object*)> construct;
    };

    template<typename T>
        requires(IsBaseOf<Object, T>)
    static T* create(mem::Allocator* allocator, const T::CreateInfo& t_info)
    {
        T* obj = allocator->object<T>();
        typename T::CreateInfo info = t_info;
        info.allocator = allocator;
        obj->init(info);
        return obj;
    }

    /*
    * The object memory allocator, Use it to allocate memory for the object.
    */
    mem::Allocator* allocator;
    InternalData data;

    void handle_event(const InputEvent& event) Function(FunctionInternal);

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

    void init(const CreateInfo& info);
    void deinit();
};

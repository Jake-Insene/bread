#pragma once
#include "core/array.h"


// A array that reuse the empty elements to create local unique identifier.
// Useful for local system resource referenced as an ID.
template<typename T, typename SlotID = u32>
struct [[nodiscard]] QueueArray
{
    static_assert(
        sizeof(T) >= sizeof(SlotID),
        "T in size must to be greater or equal to the size of SlotType"
    );

    static constexpr SlotID InvalidSlot = SlotID(-1);

    Array<T> array;
    SlotID last_free_element;

    static QueueArray from_allocator(mem::Allocator& allocator)
    {
        return QueueArray
        {
            .array = Array<T>::with_allocator(allocator),
            .last_free_element = InvalidSlot,
        };
    }

    static QueueArray with_size(mem::Allocator& allocator, const SlotID size)
    {
        return QueueArray
        {
            .array = Array<T>::with_size(allocator, size),
            .last_free_element = InvalidSlot,
        };
    }

    void destroy()
    {
        array.destroy();
    }

    [[nodiscard]] SlotID add(const T& item)
    {
        if(last_free_element != InvalidSlot)
        {
            SlotID id = last_free_element;
            SlotID* last_element = (SlotID*)&array[last_free_element];
            if(last_element[0] != InvalidSlot)
            {
                last_free_element = last_element[0];
            }

            (*(T*)last_element) = item;
            return id;
        }

        (void)array.add(item);
        return SlotID(array.count - 1);
    }

    void remove(const SlotID slot)
    {
        DebugAssert(slot < array.count, "Invalid slot");
        DebugAssert(((SlotID*)&array[slot])[0] != InvalidSlot, "Slot is already free");

        if(last_free_element == InvalidSlot)
        {
            last_free_element = slot;
            return;
        }

        SlotID* last_element = (SlotID*)&array[last_free_element];
        if(last_element[0] != InvalidSlot)
        {
            SlotID* free_element = (SlotID*)&array[slot];
            free_element[0] = last_free_element;
            last_free_element = slot;
        }
        else
        {
            SlotID* free_element = (SlotID*)&array[slot];
            free_element[0] = InvalidSlot;
            last_element[0] = last_free_element;
            last_free_element = slot;
        }
    }

    [[nodiscard]] T& get(const SlotID slot)
    {
        DebugAssert(slot < array.count, "Invalid slot");
        DebugAssert(((SlotID*)&array[slot])[0] != InvalidSlot, "Slot isn't free");
        return array[slot];
    }

    [[nodiscard]] const T& get(const SlotID slot) const
    {
        DebugAssert(slot < array.count, "Invalid slot");
        DebugAssert(((const SlotID*)&array[slot])[0] != InvalidSlot, "Slot isn't free");
        return array[slot];
    }

};

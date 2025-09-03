#pragma once
#include "collections/array.h"


// A array that reuse the empty elements to create local unique identifier.
// Useful for local system resource referenced as an ID.
template<typename T, typename SlotID = u32>
struct [[nodiscard]] QueueArray
{
    static constexpr SlotID _get_invalid_slot_value()
    {
        if constexpr (IsSame<SlotID, u64>)
        {
            return 0xEEFFEEFF'EEFFEEFFULL;
        }
        else
        {
            return 0xEEFFEEFFU;
        }
    }

    static constexpr SlotID InvalidSlot = _get_invalid_slot_value();
    static constexpr SlotID SlotBitmask = SlotID(~0);

    static_assert(
        sizeof(T) >= sizeof(SlotID),
        "T in size must to be greater or equal to the size of SlotType"
    );

    Array<T> array;
    SlotID last_free_element;
    u32 count;

    static QueueArray from_allocator(mem::Allocator& allocator)
    {
        return QueueArray
        {
            .array = Array<T>::with_allocator(allocator),
            .last_free_element = InvalidSlot,
            .count = 0,
        };
    }

    static QueueArray with_size(const mem::Allocator& allocator, const SlotID size)
    {
        return QueueArray
        {
            .array = Array<T>::with_size(allocator, size),
            .last_free_element = InvalidSlot,
            .count = 0,
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
            else
            {
                last_free_element = InvalidSlot;
            }

            count++;
            (*(T*)last_element) = item;
            return id;
        }

        (void)array.add(item);
        count++;
        return SlotID((array.count - 1) & SlotBitmask);
    }

    void remove(const SlotID slot)
    {
        DebugAssert(slot < array.count, "invalid slot");
        DebugAssert(((SlotID*)&array[slot])[0] != InvalidSlot, "slot is already free");

        count--;

        if(last_free_element == InvalidSlot)
        {
            last_free_element = slot;
            SlotID* last_element = (SlotID*)&array[last_free_element];
            last_element[0] = InvalidSlot;
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
            last_element[0] = slot;
        }
    }

    [[nodiscard]] T& get(const SlotID slot)
    {
        DebugAssert(slot < array.count, "invalid slot");
        DebugAssert(((SlotID*)&array[slot])[0] != InvalidSlot, "slot isn't free");
        return array[slot];
    }

};

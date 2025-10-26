#pragma once
#include "collections/array.h"


/*
* A array that can reuse empty slots.
* Useful for local system resource referenced as an ID.
*/
template<typename T, typename SlotID = u32>
    requires(sizeof(T) >= sizeof(SlotID))
struct [[nodiscard]] FreeList
{
    static constexpr SlotID _GetInvalidSlotValue()
    {
        if constexpr (IsSame<SlotID, u64>)
        {
            return SlotID(0xEEFFEEFF'EEFFEEFFULL);
        }
        else
        {
            return SlotID(0xEEFFEEFFU);
        }
    }

    static constexpr SlotID InvalidSlot = _GetInvalidSlotValue();
    static constexpr SlotID SlotBitmask = SlotID(~0U);

    Array<T> array;
    SlotID last_free_element;
    u32 count;

    static FreeList from_allocator(mem::Allocator& allocator)
    {
        return FreeList
        {
            .array = Array<T>::with_allocator(allocator),
            .last_free_element = InvalidSlot,
            .count = 0,
        };
    }

    static FreeList with_size(const mem::Allocator& allocator, usize size)
    {
        return FreeList
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
            SlotID* last_element = (SlotID*)&_get_element_at(last_free_element);
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

    void remove(const SlotID& slot)
    {
        DebugAssert(slot < array.count, "invalid slot");
        DebugAssert(((const SlotID*)&_get_element_at(slot))[0] != InvalidSlot, "slot is already free");

        count--;

        T& item = get(slot);
        item.~T();

        if(last_free_element == InvalidSlot)
        {
            last_free_element = slot;
            SlotID* last_element = (SlotID*)&_get_element_at(last_free_element);
            last_element[0] = InvalidSlot;
            return;
        }

        SlotID* last_element = (SlotID*)&_get_element_at(last_free_element);
        if(last_element[0] != InvalidSlot)
        {
            SlotID* free_element = (SlotID*)&_get_element_at(slot);
            free_element[0] = last_free_element;
            last_free_element = slot;
        }
        else
        {
            SlotID* free_element = (SlotID*)&_get_element_at(slot);
            free_element[0] = InvalidSlot;
            last_element[0] = slot;
        }
    }

    [[nodiscard]] T& get(const SlotID& slot)
    {
        DebugAssert(slot < array.count, "invalid slot");
        DebugAssert(((SlotID*)&_get_element_at(slot))[0] != InvalidSlot, "slot isn't free");
        return array.get(slot);
    }

    T& _get_element_at(usize index) { return array.get(index); }

};

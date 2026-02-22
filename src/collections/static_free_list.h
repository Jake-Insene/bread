#pragma once
#include "collections/static_array.h"


/*
* A array that can reuse empty slots.
* Useful for local system resource referenced as an ID.
*/
template<typename T, usize N, typename SlotID = u32>
    requires(sizeof(T) >= sizeof(SlotID))
struct [[nodiscard]] StaticFreeList
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

    StaticArray<T, N> array;
    SlotID last_free_element;
    u32 count;

    static constexpr StaticFreeList with_count(usize count)
    {
        return StaticFreeList
        {
            .array = StaticArray<T, N>::with_count(count),
            .last_free_element = InvalidSlot,
            .count = count,
        };
    }

    static constexpr StaticFreeList create()
    {
        return StaticFreeList
        {
            .last_free_element = InvalidSlot,
            .count = 0,
        };
    }

    [[nodiscard]] SlotID add(const T& item)
    {
        if (last_free_element != InvalidSlot)
        {
            SlotID id = last_free_element;
            SlotID* last_element = reinterpret_cast<SlotID*>(
                &_get_element_at(last_free_element.integer())
            );
            if (last_element[0] != InvalidSlot)
            {
                last_free_element = last_element[0];
            }
            else
            {
                last_free_element = InvalidSlot;
            }

            count++;
            *reinterpret_cast<T*>(last_element) = item;
            return id;
        }

        (void)array.add(item);
        count++;
        return SlotID((array.count - 1) & SlotBitmask.integer());
    }

    void remove(const SlotID& slot)
    {
        DebugAssert(slot.integet() < array.count, "invalid slot");
        DebugAssert(
            *reinterpret_cast<const SlotID*>(&_get_element_at(slot.integer())) != InvalidSlot,
            "slot is already free"
        );

        count--;

        T& item = get(slot);
        array.allocator.destruct(&item);

        if(last_free_element == InvalidSlot)
        {
            last_free_element = slot;
            SlotID* last_element = reinterpret_cast<SlotID*>(&_get_element_at(last_free_element.integer()));
            last_element[0] = InvalidSlot;
            return;
        }

        SlotID* last_element = reinterpret_cast<SlotID*>(&_get_element_at(last_free_element.integer()));
        if(last_element[0] != InvalidSlot)
        {
            SlotID* free_element = reinterpret_cast<SlotID*>(&_get_element_at(slot.integer()));
            free_element[0] = last_free_element;
            last_free_element = slot;
        }
        else
        {
            SlotID* free_element = reinterpret_cast<SlotID*>(&_get_element_at(slot.integer()));
            free_element[0] = InvalidSlot;
            last_element[0] = slot;
        }
    }

    [[nodiscard]] T& get(const SlotID& slot)
    {
        DebugAssert(slot < array.count, "invalid slot");
        DebugAssert(
            *reinterpret_cast<SlotID*>(&_get_element_at(slot.integer())) != InvalidSlot,
            "slot isn't free"
        );
        return array[slot];
    }

};

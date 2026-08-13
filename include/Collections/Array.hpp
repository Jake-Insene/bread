#pragma once
#include "Core/Interfaces.hpp"
#include "Collections/BaseIterator.hpp"
#include "Mem/Allocator.hpp"
#include "Mem/Utils.hpp"
#include "math/funcs.h"


namespace Collections
{

/**
* Use to iterate over an array.
*/
template<typename T>
struct [[nodiscard]] ArrayIterator : BaseIterator<T>
{
    using Type = T;

    T* base;
    usize extent;
    
    T& operator*() const { return *base; }
    T* operator->() const { return base; }

    ArrayIterator& operator++()
    {
        base++;
        extent--;
        return *this;
    }

    ArrayIterator& operator--()
    {
        base--;
        extent++;
        return *this;
    }

    [[nodiscard]] bool operator==(const ArrayIterator& it) const
    {
        return base == it.base && extent == it.extent;
    }

    ArrayIterator begin() const { return *this; }
    ArrayIterator end() const { return ArrayIterator{ .base = base + extent, .extent = 0 }; }

    [[nodiscard]] usize distance(const ArrayIterator& it) const
    {
        return (extent - it.extent);
    }
};


/**
* A collection of linear memory that contains items of type T.
* You can add/remove/modify items.
* @tparam T Element type, it can not be trivialy constructible: T value = {};
*/
template<typename T>
struct [[nodiscard]] Array final
{
    DisableCopy(Array);
    DisableMove(Array);

    static constexpr usize DefaultCapacity = 4;

    using Type = T;
    using Reference = T&;
    using Iterator = ArrayIterator<Type>;

    Mem::Allocator& allocator;
    Slice<Type> items;
    usize count;

    template<typename... TypeList>
    static constexpr Array from_list(Mem::Allocator& allocator, const TypeList... list)
    {
        static constexpr usize ListLen = sizeof...(list);
        const Type list_array[] = { list... };
        return Array(allocator, ListLen, list_array);
    }

    /**
    * @param allocator The target allocator to use to reserve memory.
    * @param initial_size Initial capacity, if 0 then it will use the default capacity.
    * @param initial_content Initial content of the array, if initial_size is 0 then the
    * initial capacity will be initial_content.len
    */
    Array(Mem::Allocator& allocator, usize initial_size, const Slice<Type>& initial_content)
    : allocator(allocator), items(), count()
    {
        usize initial_capacity = initial_size == 0 ? DefaultCapacity : initial_size;
        if(initial_size == 0)
        {
            initial_capacity = Math::max(DefaultCapacity, initial_content.len);
        }

        items = Mem::from_bytes<Type>(
            allocator.alloc(sizeof(Type) * initial_capacity, alignof(Type))
        );
        count = 0;

        if(!initial_content.null())
        {
            Mem::copy(items, initial_content);
            count = initial_content.len;
        }
    }

    ~Array()
    {
        _destruct_objects();

        if (items.ptr())
        {
            allocator.free(Mem::to_bytes(items));
            items = {};
        }
    }

    /**
    * @brief Return An iterator reflecting the current content of the array. 
    */
    template<typename Self>
    Iterator iter(this Self& self)
    {
        return
        {
            .base = self.items.items,
            .extent = self.count,
        };
    }
    
    /**
    * @brief Check if the array is empty.
    */
    [[nodiscard]] bool empty() const { return count == 0; }

    /**
    * @brief Return a reference to the element at the given index.
    */
    template<typename Self>
    [[nodiscard]] auto& get(this Self& self, usize index)
    {
        DebugAssert(index < self.count, "Index out of range");
        return self.items[index];
    }

    /**
    * @brief Returns the first element of the array.
    */
    template<typename Self>
    [[nodiscard]] auto& first(this Self& self)
    {
        DebugAssert(self.count > 0, "There is no 'first'");
        return self.items[0];
    }

    /**
    * @brief Returns the last element of the array.
    */
    template<typename Self>
    [[nodiscard]] auto& last(this Self& self)
    {
        DebugAssert(self.count > 0, "There is no 'last'");
        return self.items[self.count - 1];
    }

    /**
    * @brief Copy a new element to the end of the array.
    * This invalidates iterators.
    */
    [[nodiscard]] Type& add(const Type& item)
    requires(Core::Copyable<Type>)
    {
        _ensure_capacity(count+1);
        items[count] = item;
        return items[count++];
    }

    /**
    * @brief Place a new element to the end of the array.
    * This invalidates iterators.
    */
    template<typename... TArgs>
    [[nodiscard]] Type& emplace(TArgs&&... args)
    requires(Core::ConstructibleFrom<Type, TArgs...>)
    {
        _ensure_capacity(count+1);
        Core::Mem::Placement(items[count], Core::Forward<TArgs>(args)...);
        return items[count++];
    }

    /**
    * @brief Add all the elements in the slice.
    */
    void add_slice(const Slice<Type>& new_items)
    {
        usize begin_index = count;

        resize(count + new_items.len);
        Slice dest = items.add(begin_index);
        Mem::copy(dest, new_items);
    }

    void remove_at(const usize index)
    requires(Core::Destructible<Type>)
    {
        DebugAssert(index < count && count != 0, "Index out of range");
        Core::Mem::Destruct(items[index]);

        if (count == 1 || index == count - 1)
        {
            count--;
            return;
        }

        count--;
        Mem::copy(items.add(index), items.add(index + 1));
    }

    void remove(const Type& item)
    {
        auto it = iter().find(item);
        if (it == iter().end())
        {
            return;
        }

        remove_at(iter().distance(it));
    }

    void resize(const usize new_size)
    {
        _ensure_capacity(new_size);
        count = new_size;
    }

    void reserve(const usize new_capacity)
    {
        _ensure_capacity(new_capacity);
    }    
    
    void clear()
    {
        _destruct_objects();
        count = 0;
    }
    
    template<typename Self>
    Slice<Type> slice(this Self& self) { return self.items.slice(self.count); }

    Array copy(Mem::Allocator& copy_allocator) const
    {
        return Array(copy_allocator, 0, slice());
    }

    // Private zone
    void _ensure_capacity(const usize required_capacity)
    {
        if(items.len >= required_capacity)
        {
            return;
        }
        
        usize new_cap = items.len + (items.len / 2);
        new_cap = Math::max(new_cap, required_capacity);
        
        if(!allocator.realloc(Mem::to_bytes(items), sizeof(Type) * new_cap, alignof(Type)))
        {
            Slice new_items = Mem::from_bytes<Type>(
                allocator.alloc(sizeof(Type) * new_cap, alignof(Type))
            );

            if(items.ptr())
            {
                Mem::copy(new_items, items);
                allocator.free(Mem::to_bytes(items));
            }
            
            items = new_items;
        }
        else
        {
            items.len = new_cap;
        }
    }

    void _destruct_objects()
    requires(Core::Destructible<Type>)
    {
        for(Type& item : iter())
        {
            Core::Mem::Destruct(item);
        }
    }
};

}

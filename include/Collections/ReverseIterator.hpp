#pragma once
#include "Collections/BaseIterator.h"


template<typename T>
struct [[nodiscard]] ReverseIterator : BaseIterator<T>
{
    using Type = T;

    T* base;
    usize extent;

    template<Core::Iterable IteratorType>
    static constexpr ReverseIterator from_iter(const IteratorType& it)
    {
        return ReverseIterator{ .base = &(*(--it.end())), .extent = it.extent };
    }

    T& operator*() const { return *base; }
    T* operator->() const { return base; }

    ReverseIterator& operator++()
    {
        base--;
        extent--;
        return *this;
    }

    ReverseIterator& operator--()
    {
        base++;
        extent++;
        return *this;
    }

    [[nodiscard]] bool operator==(const ReverseIterator& it) const
    {
        return base == it.base && extent == it.extent;
    }

    ReverseIterator begin() const { return *this; }
    ReverseIterator end() const { return ReverseIterator{ .base = base - extent, .extent = 0 }; }

    [[nodiscard]] usize distance(const ReverseIterator& it) const
    {
        return (extent - it.extent);
    }
};



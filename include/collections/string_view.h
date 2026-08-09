#pragma once
#include "collections/slice.h"
#include "mem/utils.h"


template<typename T>
struct [[nodiscard]] BaseStringView : Slice<const T>
{
    using Char = T;
    using Base = Slice<const T>;

    constexpr BaseStringView() : Base(nullptr, 0) {}
    
    constexpr BaseStringView(const Char* str, usize n) : Base(str, n) {}
    
    template<usize N>
    constexpr BaseStringView(const Char(&str)[N]) : Base(str, N-1) {}

    constexpr BaseStringView(const Slice<Char>& str) : Base(str.items, str.len) {}
    
    constexpr BaseStringView(const Slice<const Char>& str) : Base(str.items, str.len) {}

    [[nodiscard]] constexpr const Char* ptr() const { return Base::items; }

    constexpr BaseStringView add(const usize offset) const
    {
        DebugAssert(ptr() && ((Base::len - offset) > 0 || (Base::len - offset) <= Base::len), "invalid offset");
        return BaseStringView(Base::items + offset, Base::len - offset);
    }

    constexpr BaseStringView sub(const usize offset) const
    {
        DebugAssert(ptr() && ((Base::len + offset) >= Base::len), "invalid offset");
        return BaseStringView(Base::items - offset, Base::len + offset);
    }
    
    [[nodiscard]] constexpr bool equals(const BaseStringView& str) const
    {
        if (Base::items == nullptr || Base::len == 0)
        {
            return false;
        }
        return Mem::compare(Slice(Base::items, str.len), str);
    }

    [[nodiscard]] constexpr bool ends_with(const BaseStringView& str) const
    {
        if(Base::len < str.len)
        {
            return false;
        }
        
        if(Mem::compare(Slice(Base::items + (Base::len - str.len), str.len), str))
        {
            return true;
        }
        
        return false;
    }

    [[nodiscard]] constexpr BaseStringView trim() const
    {
        if(Base::len == 0)
        {
            return BaseStringView();
        }

        const BaseStringView& self = *this;
        usize begin = 0;

        while(begin < self.len &&
            (self[begin] == ' '
            || self[begin] == '\n'
            || self[begin] == '\r'
            || self[begin] == '\t')
            )
        {
            begin++;
        }

        usize end = self.len - 1;

        while(self[end] == ' '
            || self[end] == '\n'
            || self[end] == '\r'
            || self[end] == '\t'
            )
        {
            end--;
        }

        return self.add(begin).slice(end - begin);
    }
};

using StringView = BaseStringView<char>;
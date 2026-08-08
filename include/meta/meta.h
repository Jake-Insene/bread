#pragma once
#include "Core/Header.h"
#include "collections/string_view.h"


namespace Meta
{

template<typename T>
constexpr StringView get_type_name()
{
    constexpr StringView function_sig = StringView(__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - 1);
    usize type_name_begin = MaxValue<usize>;
    usize type_name_end = MaxValue<usize>;
    for(usize i = 0; i < function_sig.len; i++)
    {
        if(function_sig.add(i).len == 0)
        {
            break;
        }

        if(function_sig.add(i).equals("T = "))
        {
            type_name_begin = i + 4;
        }

        if(function_sig.add(i).equals("]") && type_name_begin != MaxValue<usize>)
        {
            type_name_end = i;
        }
    }

    return function_sig.add(type_name_begin).slice(type_name_end - type_name_begin);
}

}
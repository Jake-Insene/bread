#pragma once
#include "Collections/Tuple.hpp"
#include "fmt/fmt_types.h"


namespace IO
{
struct Writer;
}


namespace Format
{

void __undef_function(...);

consteval void __fail_compile_time_on(bool condition, const char* message)
{
	if (condition)
	{
		__undef_function(message);
	}
}

struct FmtInterval
{
	usize start;
	usize len;
};

template<typename... TArgs>
struct FormatString
{
	static constexpr usize ArgumentCount = Core::GetArgumentCount<TArgs...>();
	static constexpr FormatType ArgumentTypes[ArgumentCount + 1] = { __GetFormatType<TArgs>()...};

	const char* chars;
	usize len;

	template<usize N>
	consteval FormatString(const char(&str)[N])
	: chars(str), len(N - 1)
	{
		__check_format();
	}

	consteval void __check_format()
	{
		usize i = 0;
		usize type_index = 0;

		while (i < len)
		{
			if (chars[i] == '{')
			{
				if(i + 1 != len && chars[i + 1] != '{')
				{
					Format::__fail_compile_time_on(
						i + 1 == len || chars[i + 1] != '}',
						"invalid string format"
					);
					type_index++;
				}
				i += 2;
			}
			else
			{
				i++;
			}
		}

		Format::__fail_compile_time_on(type_index != ArgumentCount, "too much/few arguments");
	}

	StringView view() const;
};

template<bool NewLine, typename... TArgs>
void format(const IO::Writer& writer, const FormatString<Core::TypeIdentity<TArgs>&&...>& fmt, TArgs&&... args);

}


#include "Collections/StringUtility.hpp"
#include "Collections/StringView.hpp"
#include "Mem/Utils.hpp"
#include "IO/Writer.hpp"

namespace Format
{

template<typename... TArgs>
StringView FormatString<TArgs...>::view() const
{
	return StringView(chars, len);
}

template<usize Base>
inline constexpr bool IsValidBase = Core::IsAnyOfValue<usize, Base, 2, 10, 16>;

template<usize Base, typename T>
requires(IsValidBase<Base>)
inline void __format_integer(const IO::Writer& writer, T arg)
{
	StringResult result = StringUtility::integer_to_string(arg, Base);
	writer.write(Mem::to_const_bytes(Slice(result.result + result.begin, result.len)));
}

template<typename T>
inline void __format_floating_point(const IO::Writer& writer, T arg, i32 decimals)
{
	StringResult result = StringUtility::fp_to_string<T>(arg, decimals);
	writer.write(Mem::to_const_bytes(Slice(result.result, result.len)));
}

template<typename T>
struct Formatter
{
	static void format_custom(const IO::Writer& writer, const T& arg);
};

template<typename T>
void __format_single_argument(const IO::Writer& writer, T&& arg)
{
	static constexpr Format::FormatType type = Format::__GetFormatType<T>();
	if constexpr(type == Format::FormatType::Bool)
	{
		static constexpr StringView true_str = "true";
		static constexpr StringView false_str = "false";
		Slice str = Mem::to_const_bytes(arg ? true_str : false_str);
		writer.write(str);
	}
	else if constexpr(type == Format::FormatType::Signed || type == Format::FormatType::Unsigned)
	{
		__format_integer<10, T>(writer, arg);
	}
	else if constexpr(type == Format::FormatType::Float32)
	{
		__format_floating_point<f32>(writer, arg, 6);
	}
	else if constexpr(type == Format::FormatType::Float64)
	{
		__format_floating_point<f64>(writer, arg, 6);
	}
	else if constexpr(type == Format::FormatType::Pointer)
	{
		__format_integer<16, usize>(writer, usize(arg));
	}
	else if constexpr(type == Format::FormatType::StringView)
	{
		writer.write(Mem::to_const_bytes(arg));
	}
	else if constexpr(type == Format::FormatType::CString)
	{
		// A CString always contains an extra byte for '\0'
		static constexpr usize len = Core::Extent<T> - 1;
		writer.write(Mem::to_const_bytes(Slice(arg, len)));
	}
	else if constexpr(type == Format::FormatType::Slice)
	{
		writer.write(Mem::to_const_bytes(StringView("[")));
		for (usize i = 0; i < arg.len; i++)
		{
			if(i != 0)
			{
				writer.write(Mem::to_const_bytes(StringView(", ")));
			}
			__format_single_argument<typename Core::RemoveReference<decltype(arg)>::Type>(writer, Move(arg[i]));
		}
		writer.write(Mem::to_const_bytes(StringView("]")));
	}
	else
	{
		Formatter<Core::RemoveCVRef<T>>::format_custom(writer, Forward<T>(arg));
	}
}

template<usize Index = 0, typename Tuple>
inline void __format_argument_at(
    const IO::Writer& writer,
    usize target,
    Tuple&& tuple)
{
    if constexpr (Index < TupleSize<Core::RemoveReference<Tuple>>)
    {
        if (target == Index)
        {
            __format_single_argument(
                writer,
                tuple.template get<Index>()
            );
            return;
        }

        __format_argument_at<Index + 1>(
            writer,
            target,
            Core::Forward<Tuple>(tuple)
        );
    }
}

template<typename... TArgs>
void __format_arguments(
    const IO::Writer& writer,
    StringView view,
    TArgs&&... args)
{
    Tuple tuple = Tuple(
        Core::Forward<TArgs>(args)...
    );

    usize start = 0;
    usize argument_index = 0;

    for (usize i = 0; i < view.len; ++i)
    {
        if (view[i] != '{')
        {
			continue;
		}

        // {{
        if (i + 1 < view.len && view[i + 1] == '{')
        {
			writer.write(
				Mem::to_const_bytes(
					view.add(start).slice(i - start)
				)
			);
			
            ++i;
			start = i;
            continue;
        }

        // {}
        if (i + 1 < view.len && view[i + 1] == '}')
        {
            writer.write(
                Mem::to_const_bytes(
                    view.add(start).slice(i - start)
                )
            );

            __format_argument_at(
                writer,
                argument_index,
                tuple
            );

            ++argument_index;
            ++i;
            start = i + 1;
        }
    }

    if (start < view.len)
    {
        writer.write(
            Mem::to_const_bytes(
                view.add(start).slice(view.len - start)
            )
        );
    }
}

template<bool NewLine, typename... TArgs>
void format(const IO::Writer& writer, const FormatString<Core::TypeIdentity<TArgs>&&...>& fmtstring, TArgs&&... args)
{
	StringView view = fmtstring.view();

	__format_arguments<TArgs...>(writer, view, Core::Forward<TArgs>(args)...);

	if constexpr(NewLine)
	{
		const u8 _character = '\n';
		const Slice new_line = Slice(&_character, 1);
		writer.write(new_line);
	}
}

}


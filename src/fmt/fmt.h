#pragma once
#include "fmt/fmt_types.h"

struct StringView;

namespace io
{
struct Writer;
}


namespace fmt
{

void __undef_function(...);

consteval void __fail_compile_time_on(bool condition, const char* message)
{
	if (condition)
		__undef_function(message);
}

struct FmtInterval
{
	usize start;
	usize len;
};

template<typename... TArgs>
struct FormatString
{
	static constexpr usize ArgumentCount = GetArgumentCount<TArgs...>();
	static constexpr FormatType ArgumentTypes[ArgumentCount + 1] = { __GetFormatType<TArgs>()...};
	static constexpr usize WriteIntervalCount = ArgumentCount + 1;

	struct FIntervalType
	{
		FmtInterval intervals[WriteIntervalCount];

		template<typename Self>
		constexpr auto& operator[](this Self& self, usize index)
		{
			return self.intervals[index];
		}
	};
	
	const char* chars;
	usize len;
	FIntervalType intervals;

	template<usize N>
	consteval FormatString(const char(&str)[N])
		: chars(str), len(N - 1), intervals(get_intervals())
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
				fmt::__fail_compile_time_on(
					i + 1 == len || type_index >= ArgumentCount || chars[i + 1] != '}',
					"invalid string format"
				);

				i += 2;
				type_index++;
			}
			else
			{
				i++;
			}
		}

		fmt::__fail_compile_time_on(type_index != ArgumentCount, "too much/few arguments");
	}

	consteval FIntervalType get_intervals()
	{
		FIntervalType intervals_l = {};

		usize i = 0;
		usize start = 0;
		usize interval_index = 0;

		while (i < len)
		{
			if (chars[i] == '{')
			{
				intervals_l[interval_index] = { start, i - start };
				i += 2;

				start = i;
				interval_index++;
			}
			else
			{
				i++;
			}
		}

		if (start != i)
		{
			intervals_l[interval_index] = { start, i - start };
		}

		return intervals_l;
	}

	StringView view() const;
};

template<bool NewLine, typename... TArgs>
void format(const io::Writer& writer, const FormatString<TypeIdentity<TArgs>&&...>& fmt, TArgs&&...);

}


#include "collections/string_utility.h"
#include "collections/string_view.h"
#include "mem/utils.h"
#include "io/writer.h"

namespace fmt
{

template<typename... TArgs>
StringView FormatString<TArgs...>::view() const
{
	return StringView(chars, len);
}

template<usize Base>
inline constexpr bool IsValidBase = IsAnyOfValue<usize, Base, 2, 10, 16>;

template<usize Base, typename T>
	requires(IsValidBase<Base>)
inline void __format_integer(const io::Writer& writer, T arg)
{
	StringResult result = StringUtility::integer_to_string<T>(arg, Base);
	writer.write(mem::to_const_bytes(Slice(result.result + result.begin, result.len)));
}

template<typename T>
inline void __format_floating_point(const io::Writer& writer, T arg, i32 decimals)
{
	StringResult result = StringUtility::fp_to_string<T>(arg, decimals);
	writer.write(mem::to_const_bytes(Slice(result.result, result.len)));
}

template<typename T>
void format_custom(const io::Writer& writer, const T& arg);

template<typename T>
void __format_single_argument(const io::Writer& writer, T&& arg)
{
	static constexpr fmt::FormatType type = fmt::__GetFormatType<T>();
	if constexpr (type == fmt::FormatType::Bool)
	{
		static constexpr StringView true_str = "true";
		static constexpr StringView false_str = "false";
		Slice<const u8> str = mem::to_const_bytes(arg ? true_str : false_str);
		writer.write(str);
	}
	else if constexpr (type == fmt::FormatType::Signed || type == fmt::FormatType::Unsigned)
	{
		__format_integer<10, T>(writer, arg);
	}
	else if constexpr (type == fmt::FormatType::Float32)
	{
		__format_floating_point<f32>(writer, arg, 6);
	}
	else if constexpr (type == fmt::FormatType::Float64)
	{
		__format_floating_point<f64>(writer, arg, 6);
	}
	else if constexpr (type == fmt::FormatType::Pointer)
	{
		__format_integer<16, usize>(writer, usize(arg));
	}
	else if constexpr (type == fmt::FormatType::String)
	{
		writer.write(mem::to_const_bytes(arg.view()));
	}
	else if constexpr (type == fmt::FormatType::StringView)
	{
		writer.write(mem::to_const_bytes(arg));
	}
	else if constexpr (type == fmt::FormatType::CString)
	{
		// A CString always contains an extra byte for '\0'
		static constexpr usize len = Extent<T> - 1;
		writer.write(mem::to_const_bytes(Slice(arg, len)));
	}
	else if constexpr (type == fmt::FormatType::Slice)
	{
		writer.write(mem::to_const_bytes(StringView("[")));
		for (usize i = 0; i < arg.len; i++)
		{
			if(i != 0)
				writer.write(mem::to_const_bytes(StringView(", ")));
			__format_single_argument<typename RemoveReference<decltype(arg)>::Type>(writer, Move(arg[i]));
		}
		writer.write(mem::to_const_bytes(StringView("]")));
	}
	else
	{
		format_custom<T>(writer, arg);
	}
}

template<usize IntervalRemain, typename... TArgs>
void __format_argument(const io::Writer& writer, const StringView view, const fmt::FormatString<TypeIdentity<TArgs>&&...>& fmtstring, TArgs&&... args)
{
	using FString = fmt::FormatString<TypeIdentity<TArgs>...>;

	if constexpr (IntervalRemain == 1)
	{
		Unused<TArgs...>(args...);
		const auto interval_range = fmtstring.intervals[FString::WriteIntervalCount - 1];
		const StringView interval = StringView(view.ptr() + interval_range.start, interval_range.len);
		writer.write(mem::to_const_bytes(interval));
	}
	else
	{
		const auto interval_range = fmtstring.intervals[FString::WriteIntervalCount - IntervalRemain];
		const StringView interval = StringView(view.ptr() + interval_range.start, interval_range.len);
		writer.write(mem::to_const_bytes(interval));

		__format_single_argument(writer, Move(GetArgument<FString::WriteIntervalCount - IntervalRemain>(Forward<TArgs>(args)...)));
		__format_argument<IntervalRemain - 1, TArgs...>(writer, view, fmtstring, Forward<TArgs>(args)...);
	}
}

template<bool NewLine, typename... TArgs>
void format(const io::Writer& writer, const FormatString<TypeIdentity<TArgs>&&...>& fmtstring, TArgs&&... args)
{
	using FString = FormatString<TypeIdentity<TArgs>...>;
	StringView view = fmtstring.view();

	if constexpr (FString::WriteIntervalCount == 1)
		writer.write(mem::to_const_bytes(view));
	else
		__format_argument<FString::WriteIntervalCount, TArgs...>(writer, view, fmtstring, Forward<TArgs>(args)...);

	if constexpr (NewLine)
	{
		u8 _character = '\n';
		Slice<const u8> new_line = { &_character, 1 };
		writer.write(new_line);
	}
}

}


#pragma once
#include "fmt/fmt_types.h"

#include <cstdio>

struct StringView;

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
	static constexpr FormatType ArgumentTypes[ArgumentCount + 1] = { __GetFormatType<TArgs>... };
	static constexpr usize WriteIntervalCount = ArgumentCount + 1;

	struct FIntervalType
	{
		FmtInterval intervals[WriteIntervalCount];

		constexpr FmtInterval& operator[](usize index)
		{
			return intervals[index];
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

template<typename... TArgs>
void format(const io::Writer& writer, FormatString<TypeIdentity<TArgs>...> fmt, TArgs...);

}


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
inline void __format_integer(const io::Writer& writer, T arg)
{
	static_assert(IsSigned<T> || IsUnsigned<T>, "expected a integer type");
	static_assert(IsValidBase<Base>, "invalid integer base");

	static constexpr usize BufferStorageSize = ConditionalValue<usize,
		Base == 10 && IsAnyOf<T, i64, u64>,
		21U,

		ConditionalValue<usize,
		Base == 10 && IsAnyOf<T, i32, u32>,
		11U,

		ConditionalValue<usize,
		Base == 10 && IsAnyOf<T, i16, u16>,
		6U,

		ConditionalValue<usize,
		Base == 10 && IsAnyOf<T, i8, u8>,
		4U,

		sizeof(T) * 8
		>>>>;

	using Unsigned = MakeUnsigned<T>;
	u8 buffer_storage[BufferStorageSize] = {};
	auto end = buffer_storage + BufferStorageSize;
	usize buffer_index = 0;

	switch (Base)
	{
	case 10:
	{
		Unsigned u = arg < 0 ? Unsigned(-arg) : Unsigned(arg);
		do
		{
			*--end = ('0' + u % 10);
			u /= 10;
			buffer_index++;
		} while (u != 0);

		if constexpr (IsSigned<T>)
		{
			if (arg < 0)
			{
				*--end = '-';
				buffer_index++;
			}
		}
	}
	break;
	case 16:
	{
		Unsigned u = arg < 0 ? Unsigned(-arg) : Unsigned(arg);
		u32 hex_digit_count = 0;

		static constexpr char HexChar[16] =
		{
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
		};
		do
		{
			*--end = HexChar[u & 0xF];
			u >>= 4;
			hex_digit_count++;
			buffer_index++;
		} while (hex_digit_count < (sizeof(T) * 8) / 4);

		if constexpr (IsSigned<T>)
		{
			if (arg < 0)
			{
				*--end = '-';
				buffer_index++;
			}
		}
	}
	break;
	default:
		break;
	}

	writer.write(Slice(buffer_storage + (BufferStorageSize - buffer_index), buffer_index));
}

inline void __format_float(const io::Writer& writer, f32 arg)
{
	char buffer[256]{};
	usize len = (usize)std::snprintf(buffer, 256, "%f", arg);
	writer.write(Slice((u8*)buffer, len));
}

inline void __format_double(const io::Writer& writer, f64 arg)
{
	char buffer[256]{};
	usize len = (usize)std::snprintf(buffer, 256, "%f", arg);
	writer.write(Slice((u8*)buffer, len));
}

template<typename T>
void __format_single_argument(const io::Writer& writer, T arg)
{
	static constexpr fmt::FormatType type = fmt::__GetFormatType<T>;
	if constexpr (type == fmt::FormatType::Bool)
	{
		if (arg)
			writer.write(mem::to_const_bytes(StringView("true")));
		else 
			writer.write(mem::to_const_bytes(StringView("false")));
	}
	else if constexpr (type == fmt::FormatType::Signed || type == fmt::FormatType::Unsigned)
	{
		__format_integer<10, T>(writer, arg);
	}
	else if constexpr (type == fmt::FormatType::Float)
	{
		__format_float(writer, arg);
	}
	else if constexpr (type == fmt::FormatType::Double)
	{
		__format_double(writer, arg);
	}
	else if constexpr (type == fmt::FormatType::Pointer)
	{
		__format_integer<16, usize>(writer, usize(arg));
	}
	else if constexpr (type == fmt::FormatType::StringView)
	{
		writer.write(mem::to_const_bytes(arg));
	}
	else if constexpr (type == fmt::FormatType::CChars)
	{
		writer.write(mem::to_const_bytes(StringView(arg, __string_len(arg))));
	}
}

template<usize IntervalRemain, typename... TArgs>
void __format_argument(const io::Writer& writer, const StringView view, fmt::FormatString<TypeIdentity<TArgs>...> fmtstring, TArgs... args)
{
	using FString = fmt::FormatString<TypeIdentity<TArgs>...>;

	if constexpr (IntervalRemain == 1)
	{
		const auto interval_range = fmtstring.intervals[FString::WriteIntervalCount - 1];
		const StringView interval = StringView(view.ptr() + interval_range.start, interval_range.len);
		writer.write(mem::to_const_bytes(interval));
	}
	else
	{
		const auto interval_range = fmtstring.intervals[FString::WriteIntervalCount - IntervalRemain];
		const StringView interval = StringView(view.ptr() + interval_range.start, interval_range.len);
		writer.write(mem::to_const_bytes(interval));

		__format_single_argument(writer, GetArgument<FString::WriteIntervalCount - IntervalRemain>(args...));
		__format_argument<IntervalRemain - 1, TArgs...>(writer, view, fmtstring, args...);
	}
}

template<typename... TArgs>
void format(const io::Writer& writer, FormatString<TypeIdentity<TArgs>...> fmtstring, TArgs... args)
{
	using FString = FormatString<TypeIdentity<TArgs>...>;
	StringView view = fmtstring.view();

	if constexpr (FString::WriteIntervalCount == 1)
		writer.write(mem::to_const_bytes(view));
	else
		__format_argument<FString::WriteIntervalCount, TArgs...>(writer, view, fmtstring, args...);

	u8 _character = '\n';
	Slice<u8> new_line = { &_character, 1 };
	writer.write(new_line);
}

}


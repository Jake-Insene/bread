#pragma once

struct StringView;
struct String;

namespace fmt
{

enum class FormatType
{
	Unknown = 0,
	Signed,
	Unsigned,
	Float,
	Double,
	Pointer,
	String,
	StringView,
	CChars,
};

template<typename T>
struct FormatArgument
{
	FormatType type;
};


template<typename T>
inline constexpr FormatType __GetFormatType = ConditionalValue<
	FormatType,
	IsSigned<T>,
	FormatType::Signed,

	ConditionalValue<FormatType,
		IsUnsigned<T>,
		FormatType::Unsigned,

	ConditionalValue<FormatType,
		IsAnyOf<T, f32>,
		FormatType::Float,

	ConditionalValue < FormatType,
		IsAnyOf<T, f64, long double>,
		FormatType::Double,
	
	ConditionalValue<FormatType,
		IsPointer<T> && !IsAnyOf<T, const char*, char*>,
		FormatType::Pointer,
				
	ConditionalValue<FormatType,
		IsAnyOf<T, StringView>,
		FormatType::StringView,

	ConditionalValue<FormatType,
		IsAnyOf<T, String>,
		FormatType::String,
	
	ConditionalValue<FormatType,
		IsAnyOf<T, const char*, char*>,
		FormatType::CChars,

		FormatType::Unknown
>>>>>>>>;


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
					i + 1 == len || type_index >= ArgumentCount || chars[i + 1] == '}',
					"invalid string format"
				);

				char t = chars[i + 1];

				switch (t)
				{
				case 'i': // Signed integer.
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::Signed, "typing inconsistency");
					break;
				case 'u': // Unsigned integer.
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::Unsigned, "typing inconsistency");
					break;
				case 'f': // Float
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::Float, "typing inconsistency");
					break;
				case 'd': // Double
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::Double, "typing inconsistency");
					break;
				case 'p': // Pointer
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::Pointer, "typing inconsistency");
					break;
				case 's': // String
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::String, "typing inconsistency");
					break;
				case 'v': // StringView
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::StringView, "typing inconsistency");
					break;
				case 'C': // CChars
					fmt::__fail_compile_time_on(ArgumentTypes[type_index] != FormatType::CChars, "typing inconsistency");
					break;
				default:
					fmt::__fail_compile_time_on(true, "unknown type format '{'");
					break;
				}

				fmt::__fail_compile_time_on(i + 2 == len || chars[i + 2] != '}', "invalid string format");

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
				i += 3;

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

template<typename... TArgs>
StringView fmt::FormatString<TArgs...>::view() const
{
	return StringView(chars, len);
}

#include "io/writer.h"

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
		u32 hdigit_count = 0;

		static constexpr char HexChar[16] =
		{
			'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
		};
		do
		{
			*--end = HexChar[u & 15];
			u >>= 4;
			hdigit_count++;
			buffer_index++;
		} while (hdigit_count < (sizeof(T) * 8) / 4);

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
	__format_integer<10, i32>(writer, i32(arg));
}

template<typename T>
void __format_single_argument(const io::Writer& writer, T arg)
{
	static constexpr fmt::FormatType type = fmt::__GetFormatType<T>;
	if constexpr (type == fmt::FormatType::Signed || type == fmt::FormatType::Unsigned)
	{
		__format_integer<10, T>(writer, arg);
	}
	else if constexpr (type == fmt::FormatType::Float)
	{
		__format_float(writer, arg);
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
void fmt::format(const io::Writer& writer, fmt::FormatString<TypeIdentity<TArgs>...> fmtstring, TArgs... args)
{
	using FString = fmt::FormatString<TypeIdentity<TArgs>...>;
	StringView view = fmtstring.view();

	if constexpr (FString::WriteIntervalCount == 1)
		writer.write(mem::to_const_bytes(view));
	else
		__format_argument<FString::WriteIntervalCount, TArgs...>(writer, view, fmtstring, args...);

	u8 _character = '\n';
	Slice<u8> new_line = { &_character, 1 };
	writer.write(new_line);
}



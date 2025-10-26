#pragma once
#include "core/templates.h"


struct StringView;
struct String;


namespace fmt
{

enum class FormatType
{
	Unknown = 0,
	Bool,
	Signed,
	Unsigned,
	Float32,
	Float64,
	Pointer,
	String,
	StringView,
	CString,
	Slice
};

template<typename T>
struct FormatArgument
{
	FormatType type;
};

template<typename T>
constexpr FormatType __GetFormatType()
{
	using TypeNoCR = RemoveConst<RemoveReference<T>>;

	if constexpr (IsSame<T, bool>)
	{
		return FormatType::Bool;
	}
	else if constexpr (IsSigned<T>)
	{
		return FormatType::Signed;
	}
	else if constexpr (IsUnsigned<T>)
	{
		return FormatType::Unsigned;
	}
	else if constexpr (IsSame<T, f32>)
	{
		return FormatType::Float32;
	}
	else if constexpr (IsSame<T, f64>)
	{
		return FormatType::Float64;
	}
	else if constexpr (IsPointer<T> && !IsAnyOf<T, const char*, char*>)
	{
		return FormatType::Pointer;
	}
	else if constexpr (IsSame<T, String>)
	{
		return FormatType::String;
	}
	else if constexpr (IsSame<T, StringView>)
	{
		return FormatType::StringView;
	}
	else if constexpr (IsArrayOf<TypeNoCR, char>)
	{
		return FormatType::CString;
	}
	else if constexpr (IsSlice<T>)
	{
		return FormatType::Slice;
	}

	return FormatType::Unknown;
}

}

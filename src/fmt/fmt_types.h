#pragma once
#include "core/templates.h"


struct StringView;
struct String;


namespace Format
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

	if constexpr (IsSame<TypeNoCR, bool>)
	{
		return FormatType::Bool;
	}
	else if constexpr (IsSigned<TypeNoCR>)
	{
		return FormatType::Signed;
	}
	else if constexpr (IsUnsigned<TypeNoCR>)
	{
		return FormatType::Unsigned;
	}
	else if constexpr (IsSame<TypeNoCR, f32>)
	{
		return FormatType::Float32;
	}
	else if constexpr (IsSame<TypeNoCR, f64>)
	{
		return FormatType::Float64;
	}
	else if constexpr (IsPointer<TypeNoCR>)
	{
		return FormatType::Pointer;
	}
	else if constexpr (IsSame<TypeNoCR, String>)
	{
		return FormatType::String;
	}
	else if constexpr (IsSame<TypeNoCR, StringView>)
	{
		return FormatType::StringView;
	}
	else if constexpr (IsArrayOf<TypeNoCR, char>)
	{
		return FormatType::CString;
	}
	else if constexpr (IsSlice<TypeNoCR>)
	{
		return FormatType::Slice;
	}

	return FormatType::Unknown;
}

}

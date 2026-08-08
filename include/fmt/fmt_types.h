#pragma once
#include "core/Templates.h"


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
	using TypeNoCR = Core::RemoveConst<Core::RemoveReference<T>>;

	if constexpr (Core::IsSame<TypeNoCR, bool>)
	{
		return FormatType::Bool;
	}
	else if constexpr (Core::IsSigned<TypeNoCR>)
	{
		return FormatType::Signed;
	}
	else if constexpr (Core::IsUnsigned<TypeNoCR>)
	{
		return FormatType::Unsigned;
	}
	else if constexpr (Core::IsSame<TypeNoCR, f32>)
	{
		return FormatType::Float32;
	}
	else if constexpr (Core::IsSame<TypeNoCR, f64>)
	{
		return FormatType::Float64;
	}
	else if constexpr (Core::IsPointer<TypeNoCR>)
	{
		return FormatType::Pointer;
	}
	else if constexpr (Core::IsSame<TypeNoCR, String>)
	{
		return FormatType::String;
	}
	else if constexpr (Core::IsSame<TypeNoCR, StringView>)
	{
		return FormatType::StringView;
	}
	else if constexpr (Core::IsArrayOf<TypeNoCR, char>)
	{
		return FormatType::CString;
	}
	else if constexpr (Core::IsSlice<TypeNoCR>)
	{
		return FormatType::Slice;
	}

	return FormatType::Unknown;
}

}

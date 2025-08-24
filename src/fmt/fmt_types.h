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
	IsSame<T, bool>,
	FormatType::Bool,
	
	ConditionalValue<
	FormatType,
	IsSigned<T>,
	FormatType::Signed,

	ConditionalValue < FormatType,
	IsUnsigned<T>,
	FormatType::Unsigned,

	ConditionalValue < FormatType,
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
	>>>>>>>>>;

}
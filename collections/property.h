#pragma once
#include "core/header.h"
#include "core/templates.h"


enum class PropertyType
{
	Unknown = 0,
	Bool,
	Int,
	UInt,
	Float,
	Pointer,
};

template<typename T>
inline constexpr PropertyType __GetPropertyType = ConditionalValue<
	PropertyType,
	IsSame<T, bool>,
	PropertyType::Bool,

	ConditionalValue<PropertyType,
	IsSigned<T> && IsInteger<T>,
	PropertyType::Int,

	ConditionalValue<PropertyType,
	IsUnsigned<T>&& IsInteger<T>,
	PropertyType::UInt,

	ConditionalValue<PropertyType,
	IsFloatingPoint<T>,
	PropertyType::Float,

	ConditionalValue<PropertyType,
	IsPointer,
	PropertyType::Pointer,

	PropertyType::Unknown
>>>>>;


union [[nodiscard]] PropertyStorage
{
	bool b;
	i64 i;
	u64 u;
	double f;
	void* p;
};

struct [[nodiscard]] PropertyValue
{
	PropertyStorage storage;

	template<typename T>
	[[nodiscard]] constexpr T get() const
	{
		if constexpr (IsSame<T, bool>)
		{
			return storage.b;
		}
		else if constexpr (IsSigned<T> && IsInteger<T>)
		{
			return storage.i;
		}
		else if constexpr (IsUnsigned<T> && IsInteger<T>)
		{
			return storage.u;
		}
		else if constexpr (IsFloatingPoint<T>)
		{
			return storage.f;
		}

		return T();
	}
};

struct Property
{
	struct Bool : PropertyValue
	{
		explicit constexpr Bool(bool v)
		{
			storage.b = v;
		}
	};
};







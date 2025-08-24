#pragma once
#include "core/header.h"
#include "core/templates.h"
#include "math/vec2.h"


enum class PropertyType
{
	Unknown = 0,
	Bool,
	Int,
	UInt,
	Float,
	Vector2,
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
	IsSame<T, Vector2>,
	PropertyType::Vector2,

	ConditionalValue<PropertyType,
	IsPointer,
	PropertyType::Pointer,

	PropertyType::Unknown
>>>>>>;


union [[nodiscard]] PropertyStorage
{
	bool b;
	i64 i;
	u64 u;
	double f;
	void* p;
	Vector2 v;

	constexpr PropertyStorage() {}
	constexpr ~PropertyStorage() {}
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
		else if constexpr (IsSame<T, Vector2>)
		{
			return storage.v;
		}

		return T();
	}

	template<typename T>
	constexpr void set(T new_value)
	{
		if constexpr (IsSame<T, bool>)
		{
			storage.b = new_value;
		}
		else if constexpr (IsSigned<T> && IsInteger<T>)
		{
			storage.i = new_value;
		}
		else if constexpr (IsUnsigned<T> && IsInteger<T>)
		{
			storage.u = new_value;
		}
		else if constexpr (IsFloatingPoint<T>)
		{
			storage.f = new_value;
		}
		else if constexpr (IsSame<T, Vector2>)
		{
			storage.v = new_value;
		}
	}
};

struct Property
{
	struct Bool : PropertyValue
	{
		explicit constexpr Bool(bool value)
		{
			storage.b = value;
		}
	};

	struct Integer : PropertyValue
	{
		template<typename T>
		explicit constexpr Integer(T value)
		{
			if constexpr (IsSigned<T>)
			{
				storage.i = value;
			}
			else
			{
				storage.u = value;
			}
		}
	};

	struct Float : PropertyValue
	{
		explicit constexpr Float(f32 value)
		{
			storage.f = value;
		}
	};

	struct Vector2 : PropertyValue
	{
		explicit constexpr Vector2(f32 x, f32 y)
		{
			storage.v = ::Vector2(x, y);
		}
	};
};







#pragma once
#include "core/templates.h"


struct StringView;

template<typename E>
StringView GetResultErrorMessage(const E& err);

template<typename E>
struct BadResult
{
	E error;

	constexpr operator E() const { return error; }
};


template<typename T, typename E>
struct [[nodiscard]] Result
{
	union
	{
		T u_value;
		E u_error;
	};

	bool has_value;

	constexpr Result(const T& v)
		: u_value(v), has_value(true)
	{}

	constexpr Result(const E& e)
		: u_error(e), has_value(false)
	{}

	constexpr Result(const BadResult<E>& e)
		: u_error(e), has_value(false)
	{}

	constexpr operator bool() const { return has_value; }

	constexpr auto value() const
	{
		if (has_value)
		{
			return u_value;
		}

		DebugAssert(false, "the result hasn't a value");
	}

	constexpr auto error() const
	{
		DebugAssert(!has_value, "the result has a value");
		return u_error;
	}

	template<typename F, typename FunctionType = FunctionDecomposed<F>>
		requires(IsSame<typename FunctionType::ReturnType, T>)
	constexpr auto or_else(F&& f) const
	{
		if (has_value)
		{
			return u_value;
		}
		
		return f(u_error);
	}
	
};



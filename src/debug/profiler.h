#pragma once
#include "os/os.h"



#define PROFILE_SCOPE(...) \
	auto Concat(func, __LINE__) = [&](f64 duration) -> void\
	{\
		__VA_ARGS__\
	};\
	using Concat(FuncType, __LINE__) = decltype(Concat(func, __LINE__));\
	struct Concat(__ProfileScopeObject, __LINE__)\
	{\
		f64 start;\
		Concat(FuncType, __LINE__) func;\
		Concat(__ProfileScopeObject, __LINE__)(Concat(FuncType, __LINE__) _func) : func(_func) { start = OS::get_time(); }\
		~Concat(__ProfileScopeObject, __LINE__)() { f64 duration = OS::get_time() - start; func(duration); }\
	};\
	Concat(__ProfileScopeObject, __LINE__) Concat(__ps, __LINE__)\
		= Concat(__ProfileScopeObject, __LINE__)(Concat(func, __LINE__));

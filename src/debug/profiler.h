#pragma once
#include "core/time.h"




#define PROFILE_SCOPE(...) \
	struct __ProfileScopeObject##__LINE__\
	{\
		f64 start;\
		__ProfileScopeObject##__LINE__() { start = Time::get_time(); }\
		~__ProfileScopeObject##__LINE__() { f64 duration = Time::get_time() - start; __VA_ARGS__ }\
	};\
	__ProfileScopeObject##__LINE__ __ps##__LINE__ = __ProfileScopeObject##__LINE__();

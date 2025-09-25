#pragma once
#include "core/macros.h"


/*
* This file contains utilities for compile time tests.
*/


#define TEST(name, ...) \
	struct Concat(TestN, __LINE__)\
	{\
		\
	};\



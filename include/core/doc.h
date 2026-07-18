#pragma once


enum class Behaviour
{
	Normal = 0,

	/*
	* Do not use this function directly, contains behaviour that is volatile.
	*/
	Internal = 0x2,

	/*
	* The use of this function is unsafe.
	*/
	Unsafe = 0x3,
};


/*
* Tag, use for visual information.
*/
#define Function(behaviour, ...)

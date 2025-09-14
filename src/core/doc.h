#pragma once


enum FunctionBehaviur
{
	FunctionNormal = 0,

	/*
	* Object function that propagates to its children.
	*/
	FunctionPropagate = 0x1,

	/*
	* Do not use this function directly, contains behaviour that is volatile.
	*/
	FunctionInternal = 0x2,

	/*
	* The use of this function is unsafe.
	*/
	FunctionUnsafe = 0x3,
};


/*
* Tag, use for visual information.
*/
#define Function(behaviur, ...)
#define RequireMark(mark_name, ...)

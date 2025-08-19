#pragma once



enum FunctionBehaviur
{
	FunctionNormal = 0,

	/*
	* Object function that propagates to its children.
	*/
	FunctionPropagate = 0x1,
};


/*
* Tag, use for visual information.
*/
#define Function(behaviur, ...)
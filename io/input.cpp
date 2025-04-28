#include "io/input.h"


bool Input::is_key_down(Key k)
{
	return data.keys[(int)k] == true;
}

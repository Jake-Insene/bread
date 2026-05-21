#include "math/vec2.h"

#include "fmt/fmt.h"


namespace Format
{

template<>
void format_custom<Vector2>(const IO::Writer& writer, const Vector2& vec)
{
	format<false>(writer, "({}, {})", vec.x, vec.y);
}

template<>
void format_custom<Vector2I>(const IO::Writer& writer, const Vector2I& vec)
{
	format<false>(writer, "({}, {})", vec.x, vec.y);
}

}
#include "gui/label.h"


void Label::init(const CreateInfo&)
{
	text = String::with_allocator(allocator);
}

void Label::render()
{
	if (font && text.count)
	{

	}
}


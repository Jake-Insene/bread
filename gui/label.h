#pragma once
#include "gui/control.h"
#include "resource/font.h"


struct Label : Control
{
	OBJECT(Label, Control);

	Font* font;
	String text;

	void init(const CreateInfo&);
	
	void render();
};

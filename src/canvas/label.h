#pragma once
#include "canvas/canvas_object.h"
#include "resource/font.h"


struct Label : CanvasObject
{
	OBJECT(Label, CanvasObject);

	Font* font;
	String text;

	void init(const CreateInfo&);
	void deinit();
	
	void render();
};

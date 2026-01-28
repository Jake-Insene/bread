#pragma once
#include "collections/array.h"
#include "gui/canvas_element.h"



struct [[nodiscard]] Canvas
{
    Array<CanvasElement> elements;
};

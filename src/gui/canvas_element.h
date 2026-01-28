#pragma once
#include "collections/string.h"



struct Canvas;


struct [[nodiscard]] CanvasElement
{
    Canvas* owner;
    String text;
};

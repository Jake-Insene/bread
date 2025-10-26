#pragma once
#include "core/header.h"


enum TextureFormat
{
    TEXTURE_FORMAT_UNKNOWN = 0,
    TEXTURE_FORMAT_RGB8,
    TEXTURE_FORMAT_RGBA8,
    TEXTURE_FORMAT_R8,
};

enum TextureType
{
    TEXTURE_UNKNOWN = 0,
    TEXTURE_2D,
};

enum TextureFilter
{
    TEXTURE_FILTER_UNKNOWN = 0,
    TEXTURE_FILTER_NEAREST,
    TEXTURE_FILTER_LINEAR,
};


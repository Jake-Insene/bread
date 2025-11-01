#pragma once
#include "collections/string_view.h"
#include "graphics/enums.h"
#include "math/vec2.h"


struct TextureCreateInfo
{
    TextureType type;
    TextureFormat format;
    TextureFilter min_filter;
    TextureFilter mag_filter;
    
    Vector2I size;
    Slice<u8> pixels;
};

struct TextureLoadInfo
{
    TextureType type;
    
    TextureFilter min_filter;
    TextureFilter mag_filter;
};

struct RenderTargetCreateInfo
{
    TextureFormat format;
    TextureFormat depth_stencil_format;
    
    Vector2I size;
};

struct MaterialCreateInfo
{};

struct MaterialCompileInfo
{
    StringView source_path;
    StringView defines;
    StringView vscode;
    StringView fscode;
};


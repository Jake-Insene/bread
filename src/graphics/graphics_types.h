#pragma once
#include "core/header.h"


struct __TextureTag {};
struct __RenderTargetTag {};
struct __RenderItemTag {};
struct __MaterialTag{};

using TextureID = ID<u32, __TextureTag>;
using RenderTargetID = ID<u32, __RenderTargetTag>;
using RenderItemID = ID<u32, __RenderItemTag>;
using MaterialID = ID<u32, __MaterialTag>;

#pragma once
#include "engine/engine.h"
#include "platform/platform_header.h"

struct AndroidEngine
{
    static inline android_app* app = nullptr;
    static inline AAssetManager* asset_manager = nullptr;

    static Egine::VTable get_vtable();
};


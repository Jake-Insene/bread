#pragma once
#include "engine/engine.h"
#include "platform/platform_header.h"

struct AndroidEngine : Engine
{
    struct InternalData
    {
        android_app* app;
        AAssetManager* asset_manager;
    };

    static inline InternalData data;
};


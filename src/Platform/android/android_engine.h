#pragma once
#include "engine/engine_runtime.h"
#include "Platform/platform_header.h"


struct AndroidEngine : EngineRuntime
{
    struct InternalData
    {
        android_app* app;
        AAssetManager* asset_manager;
    };

    static inline InternalData data;
};


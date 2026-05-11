#pragma once
#include "collections/error.h"
#include "audio/audio.h"
#include "resource/resource.h"

#include <external/dr_wav.h>


struct Sound : Resource
{
    RESOURCE(RESOURCE_SOUND, LoadFromAssets, ResourceExtensions("wav"))

    struct InternalData
    {
        u32 channels;
        Slice<i16> samples;
    } data;

	void init(const ResourceCreateInfo& info);
    void destroy();

    Error load(StringView file_path);
};
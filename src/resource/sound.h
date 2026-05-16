#pragma once
#include "audio/audio.h"
#include "collections/error.h"
#include "resource/resource.h"

#include <external/dr_wav.h>


struct Sound : Resource
{
    RESOURCE(RESOURCE_SOUND, LoadFromAssets, ResourceExtensions("wav"))

    struct InternalData
    {
        // Only use mono or stereo.
        bool mono;
        Slice<i16> samples;
    } data;

	void init(const ResourceCreateInfo& info);
    void destroy();

    Error load(StringView file_path);

    bool is_mono() const { return data.mono; }
    bool is_stereo() const { return !data.mono; }

    Audio::Frame get_frame(usize index) const;
    usize get_frame_count() const { return is_mono() ? data.samples.len : data.samples.len / 2; }
};
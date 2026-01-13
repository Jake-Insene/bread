#pragma once
#include "object/object.h"
#include "resource/sound.h"


struct AudioPlayer : Object
{
    struct InternalData
    {
        Sound* sound;

        f32 volume_cache;
        bool playing;
        bool loop = false;
    } data;

    void init(const CreateInfo& info);
    void deinit();
    void update(f32 dt);

    void play();
    void stop();

    void set_sound(Sound* new_sound) { data.sound = new_sound; }
    [[nodiscard]] Sound* get_sound() const { return data.sound; }

    void set_loop(bool enable);
    [[nodiscard]] bool get_loop() const { return data.loop; }

    void set_volume(f32 new_volume);
    [[nodiscard]] f32 get_volume()const { return data.volume_cache; }
};

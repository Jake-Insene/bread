#pragma once
#include "object/object.h"
#include "resource/sound.h"


struct AudioPlayer : Object
{
    OBJECT(AudioPlayer, Object);

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        Sound* sound;

        bool playing;
        bool loop;
    } data;

    void init(const CreateInfo&);

    void enter();
    void internal_update(f64 dt);

    void play();
    void stop();

    void set_sound(Sound* new_sound) { data.sound = new_sound; }
    [[nodiscard]] Sound* get_sound() const { return data.sound; }

    void set_loop(bool enable);
    [[nodiscard]] bool get_loop() const { return data.loop; }
};

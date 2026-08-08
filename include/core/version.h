#pragma once
#include "Core/Types.h"


namespace Core
{

/**
* Simple version utility.
*/
union Version
{
    struct
    {
        // Can be ignored sometimes.
        u16 variant;
        // if mismatch versions aren't compatible.
        u16 major;
        // if mismatch versions can be compatible.
        u16 minor;
        u16 patch;
    };

    static constexpr Version make(u16 major, u16 minor, u16 patch)
    {
        return Version
        {
            .variant = 0,
            .major = major,
            .minor = minor,
            .patch = patch,
        };
    }

    static constexpr Version make_variant(u16 variant, u16 major, u16 minor, u16 patch)
    {
        return Version
        {
            .variant = variant,
            .major = major,
            .minor = minor,
            .patch = patch,
        };
    }

    static constexpr bool compare(const Version& v1, const Version& v2)
    {
        return v1.variant == v2.variant
            && v1.major == v2.major
            && v1.minor == v2.minor
            && v1.patch == v2.patch;
    }

    static constexpr bool are_compatible(const Version& v1, const Version& v2)
    {
        return v1.major == v2.major;
    }
};

}

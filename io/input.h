#pragma once
#include "math/vec2.h"

enum InputEventType
{
    INPUT_EVENT_NONE = 0,

    INPUT_EVENT_TOUCH,
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_MOUSE_BUTTON,
};


struct InputEvent
{
    InputEventType type;
    
    template<typename T>
    const T& get() const
    {
        return reinterpret_cast<const T&>(*this);
    }
};

struct InputEventTouch : InputEvent
{
    i32 pointer = 0;
    bool pressed = false;
    Vector2 position{};
};

struct InputEventMouseMove : InputEvent
{
    Vector2 position{};
};


struct InputEventMouseButton : InputEvent
{
    Vector2 position{};
    bool left = false;
	bool right = false;
};

enum class Key
{
    None = 0,

    SHIFT = 0x10,
    
    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,

};

struct Input
{
    struct InternalData
    {
		bool keys[256];
    };

    static inline InternalData data{};

    static bool is_key_down(Key k);

};

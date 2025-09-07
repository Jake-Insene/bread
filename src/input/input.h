#pragma once
#include "math/vec2.h"

enum InputEventType
{
    INPUT_EVENT_NONE = 0,

    INPUT_EVENT_KEY,
    INPUT_EVENT_TOUCH,
    INPUT_EVENT_MOUSE_MOVE,
    INPUT_EVENT_MOUSE_BUTTON,
};

enum class Key
{
    None = 0,

    Shift = 0x10,
    Ctr = 0x11,
    Alt = 0x12,

    Left = 0x25,
    Up = 0x26,
    Right = 0x27,
    Down = 0x28,

    N0 = 0x30,
    N1 = 0x31,
    N2 = 0x32,
    N3 = 0x33,
    N4 = 0x34,
    N5 = 0x35,
    N6 = 0x36,
    N7 = 0x37,
    N8 = 0x38,
    N9 = 0x39,

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

    NumPad0 = 0x60,
    NumPad1 = 0x61,
    NumPad2 = 0x62,
    NumPad3 = 0x63,
    NumPad4 = 0x64,
    NumPad5 = 0x65,
    NumPad6 = 0x66,
    NumPad7 = 0x67,
    NumPad8 = 0x68,
    NumPad9 = 0x69,

    LeftShift = 0xA0,
    RightShift = 0xA1,
    LeftControl = 0xA2,
    RightControl = 0xA3,
    LeftAlt = 0xA4,
    RightAlt = 0xA5,
};

enum MouseButton
{
    MOUSE_BUTTON_LEFT = 0,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
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

struct InputEventKey : InputEvent
{
    bool pressed;
    Key key;
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
    MouseButton button;
    bool pressed = false;
};


enum class KeyState
{
    Released = 0,
    Pressed,

    RequestNewState,
};

struct Input
{
    struct InternalData
    {
        KeyState keys[0x1000];
        bool mouse_buttons[3];
        Vector2 mouse_position;
    };

    static inline InternalData data{};

    static KeyState get_key_state(Key k);

    static bool is_key_down(Key k);
    static bool is_key_just_down(Key k);

    static bool is_mouse_button_pressed(MouseButton button);
    static bool is_mouse_button_just_pressed(MouseButton button);

    static Vector2 get_mouse_position();
};

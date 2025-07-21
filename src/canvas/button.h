#pragma once
#include "canvas/canvas_object.h"
#include "resource/texture.h"


struct Button : CanvasObject
{
    OBJECT(Button, CanvasObject);
    
    static void _bind_vtable(VTable& vtable);

    enum ButtonState
    {
        STATE_NORMAL = 0,
        STATE_PRESSED,
        STATE_HOVERED,
    };

    // As everything in a struct is public we need to hide data
    // that should not be modified/access directly, this also
    // resolve some namespace problems.
    struct InternalData
    {
        ButtonState current_state;
    } data;

    Event<void(Object::*)()> on_pressed;
    Event<void(Object::*)()> on_released;
    
    Texture* normal_texture = nullptr;
    Texture* pressed_texture = nullptr;
    Texture* hover_texture = nullptr;
    
    void init(const CreateInfo& info);
    
    void render();
    
    void event(const InputEvent& e);

    // CanvasObject
    bool is_inside(const Vector2& pos) const;
    Rect2D get_rect() const;

    Texture* get_current_texture() const;
};

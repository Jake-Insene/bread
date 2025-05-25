#pragma once
#include "gui/control.h"
#include "io/texture.h"


struct Button : Control
{
    OBJECT(Button, Control);
    
    static void _bind_vtable(VTable& vtable);
    
    Vector2 size = Vector2(10);
    bool is_pressed;
    
    Texture* current_texture = nullptr;
    
    Texture* normal_texture = nullptr;
    Texture* pressed_texture = nullptr;
    Texture* hover_texture = nullptr;
    
    void init(const CreateInfo& info);
    
    void render();
    
    void event(const InputEvent& e);
    
    bool is_inside(const Vector2& pos) const;
};

#pragma once
#include "object/canvas_object.h"


struct Control : CanvasObject
{
    OBJECT(Control, CanvasObject);

    static void _bind_vtable(VTable& vtable);

    void init(const CreateInfo&);

    void enter();

    [[nodiscard]] bool is_control() const { return true; }
};


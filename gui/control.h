#pragma once
#include "objects/canvas_object.h"


struct Control : CanvasObject
{
    OBJECT(Control, CanvasObject);

    static void _bind_vtable(VTable& vtable);

    void init(const CreateInfo&);

    void start();

    [[nodiscard]] bool is_control() const { return true; }
};


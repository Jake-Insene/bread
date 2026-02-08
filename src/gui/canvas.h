#pragma once
#include "collections/array.h"
#include "collections/string_view.h"
#include "collections/string_map.h"
#include "gui/canvas_element.h"
#include "render/render_manager.h"



struct [[nodiscard]] Canvas
{
    using ElementEntry = StringMap<CanvasElement>::KeyValue;

    struct InternalData
    {
        mem::Allocator allocator;
        StringMap<CanvasElement> elements;
        RenderItemID render_item;
        Vector2 size;
        Vector2 position;
    };

    InternalData data;

    static Canvas create(const mem::Allocator& allocator);

    void destroy();

    CanvasElement& add_element(StringView element_id);
    [[nodiscard]] CanvasElement& get_element(StringView element_id);

    void render();
};

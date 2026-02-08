#include "gui/canvas.h"

#include "gui/canvas_element.h"


Canvas Canvas::create(const mem::Allocator& allocator)
{
    return Canvas
    {
        .data =
        {
            .allocator = allocator,
            .elements = StringMap<CanvasElement>::with_allocator(allocator),
            .render_item = RenderManager::create_item(),
            .size = Vector2(100, 100),
            .position = Vector2(),
        }
    };
}

void Canvas::destroy()
{
    for(ElementEntry& element_entry : data.elements.iter())
    {
        element_entry.second.deinit();
    }

    data.elements.destroy();
    RenderManager::destroy_item(data.render_item);
}

CanvasElement& Canvas::add_element(StringView element_id)
{
    CanvasElement element = {};
    element.init(data.allocator);
    return data.elements.insert(element_id, element);
}

CanvasElement& Canvas::get_element(StringView element_id)
{
    DebugAssert(data.elements.has(element_id), "The element doesn't exists");
    
    return data.elements.get(element_id);
}

void Canvas::render()
{
    for(ElementEntry& element : data.elements.iter())
    {
        element.second.draw(data.render_item);
    }
}

#include "physics/physics_2d.h"

#include "physics/p2d/p2d_driver.h"


void Physics2D::initialize(const mem::Allocator& allocator, Physics2D::DriverType driver)
{
    data.allocator = allocator;
    switch (driver)
    {
    case Physics2D::P2D:
        vtable = P2DDriver::get_vtable();
        break;
    default:
        FailOn(true, "Invalid physics driver");
        return;
    }

    // The implementation could need this in initialization
    data.properties = StringMap<PropertyValue>::with_size(data.allocator, 4);
    data.properties.insert("/gravity", Property::Vector2(0, -98));
    data.properties.insert("/debug_draw", Property::Bool(false));
    data.properties.insert("/tile_size", Property::Integer(64));
    data.properties.insert("/fixed_step", Property::Float(1.0f / 60.0f));

    vtable.initialize(allocator);
}

void Physics2D::shutdown()
{
    vtable.shutdown();

    data.properties.destroy();
}

void Physics2D::set_property(StringView property_name, PropertyValue new_value)
{
    data.properties.insert(property_name, new_value);
    vtable.property_change(property_name, new_value);
}

PropertyValue Physics2D::get_property(StringView property_name)
{
    return data.properties.get(property_name);
}


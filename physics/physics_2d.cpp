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

    vtable.initialize(allocator);
}


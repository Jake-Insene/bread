#include "graphics/graphics.h"

#include "graphics/gles/gles_driver.h"


void Graphics::initialize(const mem::Allocator& allocator, DriverType driver)
{
	switch (driver)
	{
	case Graphics::GLES:
		vtable = GLESDriver::get_vtable();
		break;
	default:
		FailOn(true, "Invalid graphics driver");
		break;
	}

	vtable.initialize(allocator);
}

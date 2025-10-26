#include "graphics/material.h"

#include "graphics/graphics.h"


Material Material::create()
{
	return Material
	{
		.material_id = Graphics::create_material(
			MaterialCreateInfo{}
		),
	};
}

void Material::destroy()
{
	Graphics::destroy_material(material_id);
}

void Material::compile_from_file(StringView path, StringView defines)
{
	Graphics::material_compile_from_file(material_id, path, defines);
}

void Material::compile_from_source(StringView source, StringView defines)
{
	Graphics::material_compile_from_source(material_id, source, defines);
}


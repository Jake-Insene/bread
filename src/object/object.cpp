#include "object/object.h"

#include "input/input.h"


void Object::set_group(GroupName group_name, bool value)
{
    if (value)
        data.bit_groups.set(group_name);
    else
        data.bit_groups.unset(group_name);
}

bool Object::has_group(GroupName group_name) const
{
    return data.bit_groups.is_set(group_name);
}

void Object::init(const CreateInfo& info)
{
    allocator = info.allocator;
    data.name = String::with_allocator(allocator);
}

void Object::deinit()
{
    data.name.destroy();
}


#pragma once
#include "collections/array.h"
#include "collections/ptr.h"


struct GameObject
{
    int value;
};


struct Scene
{
    struct InternalData
    {
        mem::Allocator allocator;
        Array<Ptr<GameObject>> objects;
    } data;

    static Scene create(const mem::Allocator& allocator);

    void destroy();

    Ptr<GameObject> add_object();
    void add_allocated_object(Ptr<GameObject> new_object);
};


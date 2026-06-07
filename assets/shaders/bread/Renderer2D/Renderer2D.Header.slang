#pragma once
#include "bread/Core/Structures.Header.slang"

#define DECLARE_SCENE_UNIFORM(Type, Name) \
    layout(set = 0, binding = 0) ConstantBuffer<Type> Name;

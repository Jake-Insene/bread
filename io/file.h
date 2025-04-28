#pragma once
#include "core/header.h"
#include "mem/allocator.h"

struct File
{
    static Slice<u8> read_all(mem::Allocator& allocator, StringView path);
};

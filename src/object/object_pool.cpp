#include "object/object_pool.h"

#include "debug/fail.h"
#include "mem/utils.h"



ObjectPool ObjectPool::create(const mem::Allocator& allocator)
{
    return ObjectPool
    {
        .allocator = allocator,
        .blocks = allocator.array<Block>(InitialBlockCount),
        .block_count = 0,
    };
}


void ObjectPool::destroy()
{
    for(usize i = 0; i < blocks.len; i++)
    {
        Block& block = blocks[i];
        if(block.memory.ptr())
        {
            allocator.free(block.memory);
        }
    }

    allocator.free(mem::to_bytes(blocks));
}

ObjectID ObjectPool::allocate_object(const StringView& name_tag)
{
    bool finded = false;
    usize tag_index = 0;
    for(usize i = 0; i < block_count; i++)
    {
        if(blocks[i].metadata.name.equals(name_tag))
        {
            finded = true;
            tag_index = i;
            break;
        }
    }

    if(!finded)
    {
        return ObjectID(ObjectID::InvalidID);
    }

    Block& block = blocks[tag_index];
    if(block.slot_index >= block.slot_count)
    {
        Slice<u8> new_memory = allocator.alloc(ObjectPerBlock * block.metadata.object_size, block.metadata.alignment);
        mem::copy(new_memory, block.memory);
        allocator.free(block.memory);

        block.memory = new_memory;
        block.slot_count *= 2;

    }

    usize slot_index = block.slot_index;
    block.slot_index++;

    usize block_index = tag_index;

    ObjectID obj_id = ObjectID(slot_index | (block_index << 32));
    return obj_id;
}

void ObjectPool::deallocate_object(ObjectID object_id)
{
    Unused(object_id);
    // TODO: implement object deallocation.
}

Opaque* ObjectPool::_get_object(ObjectID object_id)
{
    Block& block = blocks[_id_block(object_id)];
    u8* object_ptr = block.memory.add(block.metadata.object_size * _id_slot(object_id)).ptr();
    return reinterpret_cast<Opaque*>(object_ptr);
}


void ObjectPool::_register_object(const BlockMetadata& object_metadata)
{
    if(block_count == blocks.len)
    {
        Slice<Block> new_blocks = allocator.array<Block>(blocks.len * 2);
        mem::copy(mem::to_bytes(new_blocks), mem::to_bytes(blocks));
        allocator.free(mem::to_bytes(blocks));

        blocks = new_blocks;
    }

    Slice<u8> memory = allocator.alloc(ObjectPerBlock * object_metadata.object_size, object_metadata.alignment);

    blocks[block_count] = Block
    {
        .metadata = object_metadata,
        .memory = memory,
        .slot_count = ObjectPerBlock,
        .slot_index = 0,
    };

    block_count++;
}


#include "object/object_allocator.h"

void ObjectAllocator::initialize()
{
    data.object_allocator = {};
    usize object_size = math::next_pow2(sizeof(Object));

    for(ObjectChunk& chunk : data.chunks)
    {
        chunk = {};
        chunk.last_free_id = InvalidObjectID,
        chunk.object_size = object_size;
        object_size <<= 1;
    }
}

void ObjectAllocator::shutdown()
{
    for(ObjectChunk& chunk : data.chunks)
    {
        for(ObjectBlock& block : chunk.blocks)
        {
            if(block.bytes.ptr())
            {
                data.internal_object_allocator.free(block.bytes);
            }
        }
    }

    data.object_allocator.destroy();
}

Object* ObjectAllocator::allocate_class(const Object::Class* klass)
{
    Object* obj = allocate_object(klass);
    ObjectCallRef(obj, init,
        Object::CreateInfo
        {
            .allocator = data.object_allocator.allocator(),
        }
    );
        
    return obj;
}

Object* ObjectAllocator::allocate_object(const Object::Class* klass)
{
    // where it gonna be allocated
    usize chunk_index = math::log2(math::next_pow2(klass->class_size)) - ChunkBase;
    
    ObjectChunk& chunk = data.chunks[chunk_index];
    if(chunk.last_free_id != InvalidObjectID)
    {
        ObjectID copied_id = chunk.last_free_id;
        Object* obj = get_by_id(copied_id);
        if(obj->id != chunk.last_free_id)
        {
            chunk.last_free_id = obj->id;
        }
        else
        {
            chunk.last_free_id = InvalidObjectID;
        }
        
        klass->vtable.construct.call(obj);
        obj->klass = klass;
        obj->id = copied_id;
        return obj;
    }
    
    usize count_by_block = ObjectBlockBaseCount;
    for(usize bi = 0; bi < MaxObjectBlocks; bi++)
    {
        ObjectBlock& block = chunk.blocks[bi];
        if(block.count == 0)
        {
            allocate_new_block(block, count_by_block, chunk.object_size);
        }
        
        if(block.index < block.count)
        {
            // create the new id, see object_id.h to view the layout
            ObjectID new_id = ObjectID(block.index, bi, chunk_index);
            
            Object* new_object = 
                (Object*)(block.bytes.add(block.index * chunk.object_size).ptr());
            block.index++;

            klass->vtable.construct.call(new_object);
            new_object->id = new_id;
            new_object->klass = klass;
            return new_object;
        }
        
        count_by_block <<= 1;
    }
    
    FailOn(true, "This should not happened!");
    return nullptr;
}

void ObjectAllocator::destroy_object(Object* obj)
{
    ObjectCallRef(obj, deinit);
    FailOn(obj == nullptr || !obj->id.is_valid(), "Invalid Object");
    
    u32 chunk_index = obj->id.chunk();
    ObjectChunk& chunk = data.chunks[chunk_index];
    if(chunk.last_free_id != InvalidObjectID)
    {
        Object* last_free = get_by_id(chunk.last_free_id);
        last_free->id = chunk.last_free_id;
    }
    
    chunk.last_free_id = obj->id;
}

void ObjectAllocator::allocate_new_block(
    ObjectBlock& block, usize count, usize object_size
)
{
    block.bytes = data.internal_object_allocator.alloc(object_size * count, object_size);
    block.count = count;
}

Object* ObjectAllocator::get_by_id(ObjectID& id)
{
    FailOn(!id.is_valid(), "Invalid ObjectID");
    
    ObjectChunk& chunk = data.chunks[id.chunk()];
    return (Object*)(chunk.blocks[id.block()].bytes.add(id.slot() * chunk.object_size).ptr());
}

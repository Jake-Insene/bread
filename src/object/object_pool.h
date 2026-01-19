#pragma once
#include "core/header.h"
#include "collections/event.h"
#include "collections/string_view.h"
#include "mem/allocator.h"
#include "object/object_id.h"



/*
* ObjectID layout used by ObjectPool.
* 63-48 unused
* 47-40 unused
* 39-32 block index
* 31-0 slot index
*/
struct [[nodiscard]] ObjectPool
{
    static constexpr usize InitialBlockCount = 4;
    static constexpr usize ObjectPerBlock = 256;

    struct BlockMetadata
    {
        StringView name;
        usize object_size;
        usize alignment;

        template<typename T>
        static BlockMetadata create(const StringView& name_tag)
        {
            BlockMetadata metadata =
            {
                .name = name_tag,
                .object_size = sizeof(T),
                .alignment = alignof(T),
            };

            return metadata;
        }
    };

    struct Block
    {
        BlockMetadata metadata;
        Slice<u8> memory;
        usize slot_count;
        usize slot_index;
    };

    mem::Allocator allocator;
    Slice<Block> blocks;
    usize block_count;

    static ObjectPool create(const mem::Allocator& allocator);

    void destroy();

    template<typename T>
    void register_object(const StringView& name_tag)
    {
        _register_object(
            BlockMetadata::create<T>(name_tag)
        );
    }

    ObjectID allocate_object(const StringView& object_tag);
    void deallocate_object(ObjectID object_id);

    template<typename T>
    T* get_object(ObjectID object_id) { return _get_object(object_id)->cast<T*>(); }

    Opaque* _get_object(ObjectID object_id);
    void _register_object(const BlockMetadata& object_metadata);

    [[nodiscard]] constexpr u32 _id_slot(ObjectID object_id) const { return object_id & 0xFFFF'FFFF; }
    [[nodiscard]] constexpr u8 _id_block(ObjectID object_id) const { return (object_id >> 32) & 0xFF; }
};



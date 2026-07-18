#pragma once
#include "core/header.h"
#include "collections/map_iterator.h"
#include "collections/base_hash_map.h"
#include "collections/pair.h"
#include "mem/allocator.h"
#include "mem/utils.h"


/*
* A collection of items referenced as a key.
*/
template<Hashable K, typename V>
using HashMap = BaseHashMap<HashCode, BaseHashMapEntry<K, V, HashCode>, K, V>;

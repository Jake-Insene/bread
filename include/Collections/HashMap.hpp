#pragma once
#include "Core/Header.hpp"
#include "Collections/MapIterator.hpp"
#include "Collections/BaseHashMap.hpp"
#include "Collections/Pair.hpp"
#include "Mem/Allocator.hpp"
#include "Mem/Utils.hpp"


/*
* A collection of items referenced as a key.
*/
template<Core::Hashable K, typename V>
using HashMap = BaseHashMap<Core::HashCode, BaseHashMapEntry<K, V, Core::HashCode>, K, V>;

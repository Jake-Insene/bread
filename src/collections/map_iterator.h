#pragma once


template<typename EntryType, typename KeyValue>
struct [[nodiscard]] MapIterator
{
    using IteratorEntry = EntryType;

    IteratorEntry* entry;

    KeyValue& operator*() const { return entry->keyvalue(); }
    KeyValue* operator->() const { return &entry->keyvalue(); }

    MapIterator& operator++()
    {
        if (entry)
        {
            entry = entry->next;
        }

        return *this;
    }

    MapIterator& operator--()
    {
        if (entry)
        {
            entry = entry->prev;
        }

        return *this;
    }

    bool operator==(const MapIterator& other) const { return entry == other.entry; }
    bool operator!=(const MapIterator& other) const { return entry != other.entry; }

    MapIterator begin() const { return *this; }
    MapIterator end() const { return MapIterator(nullptr); }
};
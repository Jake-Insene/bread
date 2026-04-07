#pragma once


template<typename EntryType>
struct [[nodiscard]] MapIterator
{
    using IteratorEntry = EntryType;

    IteratorEntry* entry;

    IteratorEntry::KeyValue& operator*() const { return entry->kv; }
    IteratorEntry::KeyValue* operator->() const { return &entry->kv; }

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
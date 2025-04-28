#pragma once
#include "core/types.h"


template<typename T>
struct PropertyDefault
{
    T value;
    
    T& operator=(const T& v)
    {
        value = v;
        return value;
    }
    
    [[nodiscard]] operator T() const { return value; }
};


template<typename T>
struct PropertyPrimitive
{
    T value;
    
    T& operator=(const T& v)
    {
        value = v;
        return value;
    }
    
    [[nodiscard]] operator T() const { return value; }
    
    // We dont need to implement operato+, ect...
    // because operator T makes the work by us.
    T& operator+=(const T& v)
    {
        value += v;
        return value;
    }
    
    T& operator-=(const T& v)
    {
        value -= v;
        return value;
    }
    
    T& operator*=(const T& v)
    {
        value *= v;
        return value;
    }
    
    T& operator/=(const T& v)
    {
        value /= v;
        return value;
    }
};


template<typename T>
struct PropertyPointer
{
    T* ptr;
    
    T* operator=(T* v)
    {
        ptr = v;
        return ptr;
    }
    
    [[nodiscard]] operator T*() { return ptr; }
    [[nodiscard]] operator const T*() const { return ptr; }
    
    [[nodiscard]] T* operator->() { return ptr; }
    [[nodiscard]] const T* operator->() const { return ptr; }
};


template<typename T, T(*get)(void*, T)>
struct PropertyGet
{
    [[nodiscard]] operator T()
    {
        return get();
    }
};


struct PropertyType
{
    enum
    {
        Default,
        Primitive,
        Pointer,
    };
};


template<i32 ptype, typename T>
struct PropertySelect { using Type = PropertyDefault<T>; };

template<typename T>
struct PropertySelect<PropertyType::Primitive, T>
{
    using Type = PropertyPrimitive<T>;
};


template<typename T>
struct PropertySelect<PropertyType::Pointer, T>
{
    using Type = PropertyPointer<T>;
};


#define Property(ptype, memtype) PropertySelect<PropertyType::ptype, memtype>::Type


#define PropertyGetParent(parent_type, name)                               \
    u8* as_bytes = const_cast<u8*>(reinterpret_cast<u8 const*>(this));\
    parent_type* null_parent = nullptr;\
    usize selfOffset = reinterpret_cast<usize>(&null_parent->name);\
    parent_type* parent = reinterpret_cast<parent_type*>(as_bytes - selfOffset);\

#define PropertyGet(parent_type, type, name, getfunc)\
    [[no_unique_address]]\
    struct\
    {\
        [[nodiscard]] operator type() const\
        {\
            PropertyGetParent(parent_type, name);\
            return parent->getfunc();\
        }\
    } name
    
#define PropertySet(parent_type, type, name, setfunc)\
    [[no_unique_address]]\
    struct\
    {\
        void operator=(const type& v) const\
        {\
            PropertyGetParent(parent_type, name);\
            parent->setfunc(v);\
        }\
    } name

#define PropertyGetSet(parent_type, type, name, getfunc, setfunc)\
    [[no_unique_address]]\
    struct Property##name\
    {\
        [[nodiscard]] operator type() const\
        {\
            PropertyGetParent(parent_type, name);\
            return parent->getfunc();\
        }\
        void operator=(const type& v) const\
        {\
            PropertyGetParent(parent_type, name);\
            parent->setfunc(v);\
        }\
    } name
    
    
#define PropertyPrimitiveGetSet(parent_type, type, name, getfunc, setfunc)\
    [[no_unique_address]]\
    struct Property##name\
    {\
        [[nodiscard]] operator type() const\
        {\
            PropertyGetParent(parent_type, name);\
            return parent->getfunc();\
        }\
        void operator=(const type& v) const\
        {\
            PropertyGetParent(parent_type, name);\
            parent->setfunc(v);\
        }\
        type operator+(const type& v) const\
        {\
            return ((type)*this) + v;\
        }\
        type operator-(const type& v) const\
        {\
            return ((type)*this) - v;\
        }\
        type operator*(const type& v) const\
        {\
            return ((type)*this) * v;\
        }\
        type operator/(const type& v) const\
        {\
            return ((type)*this) / v;\
        }\
        const auto& operator+=(const type& v) const\
        {\
            operator=(((type)*this) + v);\
            return *this;\
        }\
        const auto& operator-=(const type& v) const\
        {\
            operator=(((type)*this) - v);\
            return *this;\
        }\
        const auto& operator*=(const type& v) const\
        {\
            operator=(((type)*this) * v);\
            return *this;\
        }\
        const auto& operator/=(const type& v) const\
        {\
            operator=(((type)*this) / v);\
            return *this;\
        }\
    } name

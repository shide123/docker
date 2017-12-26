#ifndef SOCKETLITE_HASH_FUN_H
#define SOCKETLITE_HASH_FUN_H
#include "SL_DataType.h"

template <typename TKey> 
struct SL_Hash 
{
};

template<>
struct SL_Hash<int64>
{
    size_t operator() (int64 x) const 
    { 
        return x; 
    }
};

template<> 
struct SL_Hash<uint64>
{
    size_t operator()(uint64 x) const
    { 
        return x;
    }
};

#endif

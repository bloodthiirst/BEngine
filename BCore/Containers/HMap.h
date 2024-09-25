#pragma once
#include "DArray.h"
#include "../Typedefs/Typedefs.h"
#include "../Allocators/Allocator.h"

struct BucketLimits
{
    size_t keys_index;
    size_t values_index;
};

template<typename K, typename V>
struct Pair
{
    K key;
    V value;
};

template<typename TKey, typename TValue>
struct HMap
{

public:

    Func<size_t, TKey> hasher;
    Func<bool, TKey, TKey> comparer;

    Allocator allocator;

    DArray<DArray<BucketLimits>> bucket_per_hash;

    DArray<TKey> all_keys;
    DArray<TValue> all_values;

    size_t capacity;
    size_t max_bucket_count;
    size_t count;

private:

    static bool BucketComparer( BucketLimits a, BucketLimits b )
    {
        return (a.keys_index == b.keys_index) && (a.values_index == b.values_index);
    }

public:

    /// <summary>
    /// Initialize the dictionary with "capacity" being the number of bucket and "maxBucketCount" beings the initial size of each bucket
    /// </summary>
    static bool Create( HMap* out_map, Allocator alloc, size_t capacity, size_t maxBucketCount, Func<size_t, TKey> hasher, Func<bool, TKey, TKey> comparer )
    {
        out_map->allocator = alloc;
        out_map->max_bucket_count = maxBucketCount;
        out_map->capacity = capacity;
        out_map->hasher = hasher;
        out_map->comparer = comparer;
        out_map->count = 0;
        out_map->bucket_per_hash = {};

        DArray<DArray<BucketLimits>>::Create( capacity, &out_map->bucket_per_hash, alloc );

        for ( size_t i = 0; i < capacity; i++ )
        {
            DArray<BucketLimits> b = {};
            DArray<BucketLimits>::Create( capacity, &b, alloc );
            DArray<DArray<BucketLimits>>::Add( &out_map->bucket_per_hash, b );
        }

        out_map->all_keys = {};
        out_map->all_values = {};

        DArray<TKey>::Create( capacity, &out_map->all_keys, alloc );
        DArray<TValue>::Create( capacity, &out_map->all_values, alloc );

        return true;
    }

    static bool Destroy( HMap* in_map )
    {
        for ( size_t i = 0; i < in_map->bucket_per_hash.size; i++ )
        {
            DArray<BucketLimits>::Destroy( &in_map->bucket_per_hash.data[i] );
        }

        DArray<DArray<BucketLimits>>::Destroy( &in_map->bucket_per_hash );
        DArray<TKey>::Destroy( &in_map->all_keys );
        DArray<TValue>::Destroy( &in_map->all_values );

        *in_map = {};

        return true;
    }

    static bool TryAdd( HMap* in_map, TKey key, TValue value, size_t* out_index )
    {
        if ( (in_map->count + 1) > in_map->capacity )
        {
            Resize( in_map, (in_map->count * 2) + 1 );
        }

        size_t hash = in_map->hasher( key );
        size_t index = hash % in_map->capacity;

        // get the bucket using the mod'ed hash
        DArray<BucketLimits>* buckets = &in_map->bucket_per_hash.data[index];

        // check if the key already exists
        for ( size_t i = 0; i < buckets->size; ++i )
        {
            BucketLimits b = buckets->data[i];

            if ( in_map->comparer( in_map->all_keys.data[b.keys_index], key ) )
            {
                *out_index = -1;
                return false;
            }
        }

        size_t keyIndex = in_map->all_keys.size;
        DArray<TKey>::Add( &in_map->all_keys, key );

        size_t valueIndex = in_map->all_values.size;
        DArray<TValue>::Add( &in_map->all_values, value );

        BucketLimits new_bucket = {};
        new_bucket.keys_index = keyIndex;
        new_bucket.values_index = valueIndex;

        if(out_index != nullptr)
        {
            *out_index = keyIndex;
        }
        
        DArray<BucketLimits>::Add( buckets, new_bucket );
        in_map->count++;

        return true;
    }

    static bool TryGet( HMap* in_map, TKey key, TValue** out_val )
    {
        size_t hash = in_map->hasher( key );
        size_t index = hash % in_map->capacity;

        // get the bucket using the mod'ed hash
        DArray<BucketLimits>* bucket = &in_map->bucket_per_hash.data[index];

        // check if the key already exists
        for ( size_t i = 0; i < bucket->size; ++i )
        {
            BucketLimits b = bucket->data[i];

            if ( in_map->comparer( in_map->all_keys.data[b.keys_index], key ) )
            {
                *out_val = &in_map->all_values.data[b.values_index];
                return true;
            }
        }

        *out_val = {};
        return false;
    }

    /// <summary>
    /// Returns the content of the HMap as Key-Value pairs
    /// </summary>
    static void GetAll( HMap<TKey, TValue>* in_map, DArray<Pair<TKey, TValue>>* in_result )
    {
        for ( size_t i = 0; i < in_map->bucket_per_hash.size; ++i )
        {
            DArray<BucketLimits> b = in_map->bucket_per_hash.data[i];

            for ( size_t j = 0; j < b.size; ++j )
            {
                BucketLimits indicies = b.data[j];

                Pair<TKey, TValue> pair = {};
                pair.key = in_map->all_keys.data[indicies.keys_index];
                pair.value = in_map->all_values.data[indicies.values_index];

                DArray<Pair<TKey, TValue>>::Add( in_result, pair );
            }
        }
    }

    static bool TryRemove( HMap* in_map, TKey key, TValue* out_removed )
    {
        size_t hash = in_map->hasher( key );
        size_t index = hash % in_map->capacity;

        // get the bucket using the mod'ed hash
        DArray<BucketLimits>* bucket = &in_map->bucket_per_hash.data[index];

        size_t limit_index = {};
        bool found = false;

        // check if the key already exists
        for ( size_t i = 0; i < bucket->size; ++i )
        {
            BucketLimits b = bucket->data[i];

            if ( !in_map->comparer( in_map->all_keys.data[b.keys_index], key ) )
            {
                continue;
            }

            limit_index = i;
            found = true;
            break;
        }

        if ( !found )
        {
            *out_removed = {};
            return false;
        }

        BucketLimits to_remove = bucket->data[limit_index];

        *out_removed = in_map->all_values.data[to_remove.values_index];

        DArray<BucketLimits>::RemoveAt( bucket, limit_index );
        DArray<TKey>::RemoveAt( &in_map->all_keys, to_remove.keys_index );
        DArray<TValue>::RemoveAt( &in_map->all_values, to_remove.values_index );

        for ( size_t i = 0; i < in_map->bucket_per_hash.size; ++i )
        {
            DArray<BucketLimits>* b = &in_map->bucket_per_hash.data[i];

            for ( size_t j = 0; j < b->size; ++j )
            {
                BucketLimits* curr_b = &b->data[j];

                size_t key_pushback = (size_t) curr_b->keys_index > to_remove.keys_index;
                size_t val_pushback = (size_t) curr_b->values_index > to_remove.values_index;

                curr_b->keys_index -= key_pushback;
                curr_b->values_index -= val_pushback;
            }
        }

        in_map->count--;

        return true;
    }


    static void Resize( HMap* in_map, size_t capacity )
    {
        int32_t diff = (uint32_t)capacity - (uint32_t)in_map->count;
        assert( diff >= 0 );

        // TODO : actually expand the map
        Allocator alloc = ArenaAllocator::Create( &CoreContext::core_arena );
        size_t start_offset = CoreContext::core_arena.offset;
        {
            DArray<Pair<TKey, TValue>> pairs = {};
            DArray<Pair<TKey, TValue>>::Create( in_map->count, &pairs, alloc );

            // save the old values
            HMap<TKey, TValue>::GetAll( in_map, &pairs );

            in_map->count = pairs.size;
            in_map->capacity = capacity;

            for ( size_t i = 0; i < in_map->bucket_per_hash.size; ++i )
            {
                DArray<BucketLimits>::Clear( &in_map->bucket_per_hash.data[i] );
            }

            for ( size_t i = 0; i < diff; ++i )
            {
                DArray<BucketLimits> new_bucket = {};
                DArray<BucketLimits>::Create( 0, &new_bucket, in_map->allocator );
                DArray<DArray<BucketLimits>>::Add( &in_map->bucket_per_hash, new_bucket );
            }

            DArray<TKey>::Clear( &in_map->all_keys );
            DArray<TValue>::Clear( &in_map->all_values );

            // add the key-value back into the new resized hmap
            for ( size_t i = 0; i < pairs.size; ++i )
            {
                TKey key = pairs.data[i].key;
                TValue value = pairs.data[i].value;

                size_t hash = in_map->hasher( key );
                size_t index = hash % in_map->capacity;

                // get the bucket using the mod'ed hash
                DArray<BucketLimits>* buckets = &in_map->bucket_per_hash.data[index];

                size_t keyIndex = in_map->all_keys.size;
                DArray<TKey>::Add( &in_map->all_keys, key );

                size_t valueIndex = in_map->all_values.size;
                DArray<TValue>::Add( &in_map->all_values, value );

                BucketLimits new_bucket = {};
                new_bucket.keys_index = keyIndex;
                new_bucket.values_index = valueIndex;

                DArray<BucketLimits>::Add( buckets, new_bucket );
            }
        }
        CoreContext::core_arena.offset = start_offset;
    }
};
#pragma once
#include "DArray.h"
#include "../Typedefs/Typedefs.h"
#include "../Allocators/Allocator.h"

struct BucketLimits
{
    size_t keys_index;
    size_t values_index;
    BucketLimits* next_ptr;
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

    DArray<BucketLimits> bucket_per_hash;

    DArray<TKey> all_keys;
    DArray<TValue> all_values;

    size_t capacity;
    size_t count;

private:

    static bool BucketComparer( BucketLimits a, BucketLimits b )
    {
        return (a.keys_index == b.keys_index) && (a.values_index == b.values_index);
    }

    static BucketLimits EmptyBucket()
    {
        BucketLimits empty = {};
        empty.keys_index = -1;
        empty.values_index = -1;
        empty.next_ptr = nullptr;

        return empty;
    }

public:

    /// <summary>
    /// Initialize the dictionary with "capacity" being the number of bucket and "maxBucketCount" beings the initial size of each bucket
    /// </summary>
    static bool Create( HMap* out_map, Allocator alloc, size_t capacity, Func<size_t, TKey> hasher, Func<bool, TKey, TKey> comparer )
    {
        *out_map = {};
        out_map->allocator = alloc;
        out_map->capacity = capacity;
        out_map->hasher = hasher;
        out_map->comparer = comparer;
        out_map->count = 0;

        DArray<BucketLimits>::Create( capacity, &out_map->bucket_per_hash, alloc );
        DArray<TKey>::Create( capacity, &out_map->all_keys, alloc );
        DArray<TValue>::Create( capacity, &out_map->all_values, alloc );

        // init all to empty buckets
        for(size_t i = 0; i < capacity; ++i)
        {
            BucketLimits empty = EmptyBucket();

            DArray<BucketLimits>::Add(&out_map->bucket_per_hash , empty);
        }

        return true;
    }

    static bool Destroy( HMap* in_map )
    {
        DArray<BucketLimits>::Destroy( &in_map->bucket_per_hash );
        DArray<TKey>::Destroy( &in_map->all_keys );
        DArray<TValue>::Destroy( &in_map->all_values );

        *in_map = {};

        return true;
    }

    static bool TryAdd( HMap* in_map, TKey key, TValue value, size_t* out_idx )
    {
        if ( (in_map->count + 1) > in_map->capacity )
        {
            Resize( in_map, (in_map->count * 2) + 1 );
        }

        size_t hash = in_map->hasher( key );
        size_t index = hash % in_map->capacity;

        // get the bucket using the mod'ed hash
        BucketLimits* root_bucket = &in_map->bucket_per_hash.data[index];
        BucketLimits* parent_bucket = nullptr;

        size_t key_idx = {};

        // if it's first time filling bucket , then just add and skip
        if(root_bucket->keys_index == -1 && root_bucket->values_index == -1)
        {
            // add pair
            key_idx = in_map->all_keys.size;
            DArray<TKey>::Add( &in_map->all_keys, key );

            size_t value_idx = in_map->all_values.size;
            DArray<TValue>::Add( &in_map->all_values, value );

            BucketLimits new_bucket = {};
            new_bucket.keys_index = key_idx;
            new_bucket.values_index = value_idx;
            new_bucket.next_ptr = nullptr;

            *root_bucket = new_bucket;

            goto end;
        }

        // else , iterate through the buckets
        // the goal is to check if the key already exits
        // AND to find the place for the next bucket
        while(root_bucket != nullptr)
        {
            TKey curr_key = in_map->all_keys.data[root_bucket->keys_index];
            bool found = in_map->comparer( curr_key, key );
            
            if ( found )
            {
                *out_idx = -1;
                return false;
            }

            parent_bucket = root_bucket;
            root_bucket = root_bucket->next_ptr;
        }       
        
        // add pair
        {
            key_idx = in_map->all_keys.size;
            DArray<TKey>::Add( &in_map->all_keys, key );

            size_t value_idx = in_map->all_values.size;
            DArray<TValue>::Add( &in_map->all_values, value );

            BucketLimits new_bucket = {};
            new_bucket.keys_index = key_idx;
            new_bucket.values_index = value_idx;
            new_bucket.next_ptr = nullptr;

            size_t new_bucket_idx = in_map->bucket_per_hash.size;
            DArray<BucketLimits>::Add(&in_map->bucket_per_hash , new_bucket);
            parent_bucket->next_ptr = &in_map->bucket_per_hash.data[new_bucket_idx];
        }

end:
        in_map->count++;
        
        if(out_idx != nullptr)
        {
            *out_idx = key_idx;
        }
        
        return true;
    }

    static bool TryGet( HMap* in_map, TKey key, TValue** out_val )
    {
        size_t hash = in_map->hasher( key );
        size_t index = hash % in_map->capacity;

        // get the bucket using the mod'ed hash
        BucketLimits* bucket = &in_map->bucket_per_hash.data[index];

        if(bucket->keys_index == -1 && bucket->values_index == -1)
        {
            goto end;
        }

        // check if the key already exists
        do
        {
            TKey curr_key = in_map->all_keys.data[bucket->keys_index];

            if ( !in_map->comparer( curr_key, key ) )
            {
                bucket = bucket->next_ptr;
                continue;
            }

            TValue* value_ptr = &in_map->all_values.data[bucket->values_index];
            *out_val = value_ptr;
            return true;
        }
        while (bucket != nullptr);
        
end:
        *out_val = {};
        return false;
    }

    /// <summary>
    /// Returns the content of the HMap as Key-Value pairs
    /// </summary>
    static void GetAll( HMap<TKey, TValue>* in_map, DArray<Pair<TKey, TValue>>* in_result )
    {
        if(in_result->capacity < in_map->count)
        {
            DArray<Pair<TKey,TValue>>::Resize(in_result , in_map->count);
        }

        size_t idx = 0;

        while(in_result->size < in_map->count)
        {
            BucketLimits curr_bucket = in_map->bucket_per_hash.data[idx];

            if(curr_bucket.keys_index != -1 && curr_bucket.values_index != -1)
            {
                Pair<TKey, TValue> pair = {};
                pair.key = in_map->all_keys.data[curr_bucket.keys_index];
                pair.value = in_map->all_values.data[curr_bucket.values_index];

                DArray<Pair<TKey, TValue>>::Add( in_result, pair );
            }

            idx++;
        }       
    }

    static bool TryRemove( HMap* in_map, TKey key, TValue* out_removed )
    {
        size_t hash = in_map->hasher( key );
        size_t index = hash % in_map->capacity;

        // get the bucket using the mod'ed hash
        BucketLimits* bucket_to_remove = &in_map->bucket_per_hash.data[index];
        BucketLimits bucket_to_remove_val = *bucket_to_remove;

        bool found = false;

        // if the root is already empty , early exit
        if(bucket_to_remove->keys_index == -1 && bucket_to_remove->values_index == -1)
        {
            *out_removed = {};
            return false;
        }

        // check if the key already exists
        while(bucket_to_remove != nullptr)
        {
            TKey curr_key = in_map->all_keys.data[bucket_to_remove->keys_index];
            found = in_map->comparer( curr_key, key );
            
            if ( found )
            {
                break;
            }

            bucket_to_remove = bucket_to_remove->next_ptr;
        }

        if ( !found )
        {
            *out_removed = {};
            return false;
        }
    
        *out_removed = in_map->all_values.data[bucket_to_remove_val.values_index];

        if(bucket_to_remove->next_ptr == nullptr)
        {
            *bucket_to_remove = EmptyBucket();
        }
        else
        {
            *bucket_to_remove = *bucket_to_remove->next_ptr;
        }

        DArray<TKey>::RemoveAt(&in_map->all_keys , bucket_to_remove_val.keys_index );
        DArray<TValue>::RemoveAt(&in_map->all_values , bucket_to_remove_val.values_index );

        for(size_t i = 0; i < in_map->count; ++i)
        {
            BucketLimits* curr = &in_map->bucket_per_hash.data[i];

            if(curr->keys_index == -1 && curr->values_index == -1)
            {
                continue;
            }
            if(curr->keys_index > bucket_to_remove_val.keys_index)
            {
                curr->keys_index--;
            }
            
            if(curr->values_index > bucket_to_remove_val.values_index)
            {
                curr->values_index--;
            }
        }
        
        in_map->count--;

        return true;
    }


    static void Resize( HMap* in_map, size_t new_capacity )
    {
        int32_t diff = (uint32_t)new_capacity - (uint32_t)in_map->count;
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
            in_map->capacity = new_capacity;

            // TODO : find a way to resize without "keeping the data" since it will be reinserted anyways
            {
                DArray<BucketLimits>::Resize( &in_map->bucket_per_hash , new_capacity );
                DArray<TKey>::Resize( &in_map->all_keys , new_capacity );
                DArray<TValue>::Resize( &in_map->all_values , new_capacity );

                DArray<TKey>::Clear( &in_map->all_keys );
                DArray<TValue>::Clear( &in_map->all_values );
                DArray<BucketLimits>::Clear( &in_map->bucket_per_hash );

                // init all to empty buckets
                for(size_t i = 0; i < new_capacity; ++i)
                {
                    BucketLimits empty = EmptyBucket();

                    DArray<BucketLimits>::Add(&in_map->bucket_per_hash , empty);
                }

            }

            // add the key-value back into the new resized hmap
            for ( size_t i = 0; i < pairs.size; ++i )
            {
                Pair<TKey,TValue> p = pairs.data[i];
                HMap<TKey,TValue>::TryAdd(in_map , p.key , p.value , nullptr);
            }
        }
        CoreContext::core_arena.offset = start_offset;
    }
};
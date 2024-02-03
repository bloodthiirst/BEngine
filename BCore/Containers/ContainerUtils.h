#pragma once
#include "../Typedefs/Typedefs.h"
#include "DArray.h"

struct ContainerUtils
{
    template <typename T>
    static void Sort( T* data, size_t start, size_t count, Func<bool, T, T> comparer )
    {
        size_t end = start + count;

        for ( size_t i = start; i < end; ++i )
        {
            size_t max_index = i;
            T max_val = data[max_index];

            for ( size_t j = i + 1; j < end; ++j )
            {
                T other_val = data[j];

                if ( comparer( max_val, other_val ) )
                {
                    max_index = j;
                    max_val = other_val;
                }
            }

            // swap
            T tmp = data[i];
            data[i] = max_val;
            data[max_index] = tmp;
        }
    };

    template <typename T>
    static void Where( T* data, size_t start, size_t count, Func<bool, T> filter, DArray<T*> in_result )
    {
        size_t end = start + count;

        for ( size_t i = start; i < end; ++i )
        {
            if ( filter( data[i] ) )
            {
                DArray<T*>::Add( &in_result, & data[i] );
            }
        }
    };


    template <typename T>
    static bool IndexOf( T* data, size_t start, size_t count, T value, size_t* out_index )
    {
        size_t end = start + count;

        for ( size_t i = start; i < end; ++i )
        {
            if ( data[i] == value )
            {
                *out_index = i;
                return true;
            }
        }

        return false;
    };


    template <typename T>
    static bool IndexOf( T* data, size_t start, size_t count, Func<bool, T> filter, size_t* out_index )
    {
        size_t end = start + count;

        for ( size_t i = start; i < end; ++i )
        {
            if ( filter( data[i] ) )
            {
                *out_index = i;
                return true;
            }
        }

        return false;
    };


};
#pragma once
#include "StringView.h"
#include "../Allocators/Allocator.h"

/// <summary>
/// <para>A wrapper around an editable chunk of memory used to represent strings</para>
/// <para>A StringBuffer supports editing and manipulating the memory used for the string</para>
/// </summary>
struct StringBuffer
{
    /// <summary>
    /// The allocator used to manipulate the memory associated with the string
    /// </summary>
    Allocator alloc;

    union
    {
        /// <summary>
        /// Get a view of the StringBuffer
        /// </summary>
        StringView view;

        struct
        {
            /// <summary>
            /// Pointer to the start of the string
            /// </summary>
            char* buffer;

            /// <summary>
            /// The length of the string not including the termination character
            /// </summary>
            size_t length;
        };

    };

    static StringBuffer Create(size_t length, Allocator allocator);

    static StringBuffer Create(const char* str, size_t from, size_t to, Allocator allocator);

    static StringBuffer Create(const char* str, Allocator allocator);

    static StringBuffer Create(StringView str, Allocator allocator);
    static void Destroy( StringBuffer* str);

    static void Resize(StringBuffer* str, size_t size);
};
#pragma once
#include "../Defines/Defines.h"

struct Allocator;

/// <summary>
/// <para>A wrapper around strings that represents a readonly view of them</para>
/// <para>The buffer is not guaranteed to be null-terminated , it can be a view into a chunch int the middle of another string </para>
/// </summary>
struct CORE_API StringView
{
    /// <summary>
    /// Pointer to the start of the string
    /// </summary>
    const char* buffer;

    /// <summary>
    /// The length of the string not including the termination character
    /// </summary>
    size_t length;

    /// <summary>
    /// Create a StringView from a constant C string
    /// </summary>
    /// <param name="str"></param>
    /// <returns></returns>
    static StringView Create(const char* str);

    /// <summary>
    /// Create a C string (with termination character) from the view
    /// </summary>
    /// <param name="str_view"></param>
    /// <param name="alloc"></param>
    /// <returns></returns>
    static char* ToCString(StringView str_view, Allocator alloc);

    StringView( const char* c_str )
    {
        buffer = c_str;
        length = 0;

        while ( c_str[length] != '\0' )
        {
            ++length;
        }
    }

    StringView() = default;
};


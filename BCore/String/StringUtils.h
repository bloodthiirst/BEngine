#pragma once
#include <cassert>
#include <memory>
#include "../Allocators/Allocator.h"
#include "StringBuffer.h"
#include "StringView.h"
#include "StringBuffer.h"

class StringUtils
{

private:

    static void ConcatInternal( StringBuffer buffer, size_t curr_index, StringView start )
    {
        for (size_t i = 0; i < start.length; ++i)
        {
            buffer.buffer[curr_index + i] = start.buffer[i];
        }
    }

    template<typename ...Args>
    static void ConcatInternal( StringBuffer buffer, size_t curr_index, StringView start, Args... more )
    {
        for (size_t i = 0; i < start.length; ++i)
        {
            buffer.buffer[curr_index + i] = start.buffer[i];
        }

        ConcatInternal(buffer , curr_index + start.length, more...);
    }

    static size_t GetLength( StringView str )
    {
        return str.length;
    }
public:

    template<typename ...Args>
    static StringBuffer Concat( Allocator alloc, size_t total_length, StringView start, Args... args )
    {
        StringBuffer result = StringBuffer::Create( total_length, alloc );

        ConcatInternal( result, 0, start, args... );

        return result;
    }

    template<typename ...Args>
    static StringBuffer Concat( Allocator alloc, StringView start, Args... args )
    {
        size_t total_size = GetLength(start , args...);
        return Concat(alloc , total_size , start , args...);
    }



    template<typename ...Args>
    static size_t GetLength( StringView first, Args... more )
    {
        StringView args[] = { more... };

        size_t count = sizeof(args) / sizeof(StringView);

        size_t res = first.length;

        for (size_t i = 0; i < count ; ++i)
        {
            res += args[i].length;
        }

        return res;
    }

    static bool Compare( const char* a, const char* b, size_t length )
    {
        size_t curr_len = 0;
        bool are_equal = true;
        while ((curr_len < length) && (*a != '\0') && (are_equal = *(a++) == *(b++)))
        {
            curr_len++;
        }

        return are_equal;
    }

    static size_t ReplaceChar( char* string, const char to_replace, const char replace_with )
    {
        size_t curr_index = 0;
        size_t occurence = 0;
        char curr_char;

        while ((curr_char = string[curr_index]) != '\0')
        {
            if (curr_char == to_replace)
            {
                string[curr_index] = replace_with;
                occurence++;
            }

            ++curr_index;
        }

        return occurence;
    }

    static StringBuffer Replace( const StringView original, StringView to_replace, StringView replace_with, Allocator allocator )
    {
        size_t len = original.length;
        size_t old_char_len = to_replace.length;
        size_t new_char_len = replace_with.length;

        size_t length_needed = len * sizeof( size_t );
        Allocator temp_alloc = STACK_ALLOC( length_needed );
        size_t* and_indicies = (size_t*)temp_alloc.alloc( temp_alloc, length_needed );
        size_t and_count = 0;
        for (size_t i = 0; i < len; ++i)
        {
            if (Compare( (original.buffer + i), to_replace.buffer, old_char_len ))
            {
                and_indicies[and_count++] = i;
            }
        }



        StringBuffer new_string = StringBuffer::Create( len + and_count, allocator );
        size_t old_string_start = 0;
        size_t new_string_start = 0;

        for (size_t i = 0; i < and_count; ++i)
        {
            size_t index = and_indicies[i];
            size_t size_to_copy = index - old_string_start;

            memcpy( new_string.buffer + new_string_start, original.buffer + old_string_start, size_to_copy );

            old_string_start += size_to_copy + old_char_len;
            new_string_start += size_to_copy;

            memcpy( new_string.buffer + new_string_start, replace_with.buffer, new_char_len );

            new_string_start += new_char_len;
        }

        memcpy( new_string.buffer + new_string_start, original.buffer + old_string_start, len - old_string_start );

        return new_string;
    }

    static StringBuffer GetFilename( StringView path, Allocator allocator )
    {
        _ASSERT( path.buffer != nullptr );

        size_t last_anti_slash = -1;
        size_t total_length = 0;
        char curr_char;

        while ((total_length < path.length) && (curr_char = path.buffer[total_length++]) != '\0')
        {
            if (curr_char == '\\')
            {
                last_anti_slash = total_length;
            }
        }

        return StringBuffer::Create( path.buffer, last_anti_slash + 1, total_length - 1, allocator );
    }

    static StringBuffer ToString(int32_t value , Allocator alloc)
    {
        char* mem = (char*) alloc.alloc(alloc, 32);
        _itoa_s(value, mem, 32 , 10);

        return StringBuffer::Create(mem, alloc);
    }

    static StringBuffer GetExtension( StringView path, Allocator alloc )
    {
        _ASSERT( path.buffer != nullptr );

        size_t last_dot = -1;
        size_t total_length = 0;
        char curr_char;

        while ((total_length < path.length) && (curr_char = path.buffer[total_length++]) != '\0')
        {
            if (curr_char == '.')
            {
                last_dot = total_length;
            }
        }
        return StringBuffer::Create( path.buffer, last_dot, total_length - last_dot, alloc );
    }


    static StringBuffer GetFolder( StringView path, Allocator alloc )
    {
        _ASSERT( path.buffer != nullptr );

        size_t last_anti_slash = -1;
        size_t total_length = 0;
        char curr_char;

        while ( (total_length < path.length) && (curr_char = path.buffer[total_length++]) != '\0')
        {
            if (curr_char == '\\')
            {
                last_anti_slash = total_length;
            }
        }
        return StringBuffer::Create( path.buffer , 0 , last_anti_slash, alloc );
    }

};
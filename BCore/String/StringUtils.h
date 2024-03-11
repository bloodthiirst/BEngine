#pragma once
#include <cassert>
#include "../Defines/Defines.h"
#include "../Allocators/Allocator.h"
#include "../Containers/DArray.h"
#include "StringBuffer.h"
#include "StringView.h"
#include "StringBuffer.h"
#include "ToString.h"
#include "../Context/CoreContext.h"

class CORE_API StringUtils
{
private:

    template<typename T, typename ...Args>
    static void FormatAccumulator( DArray<StringBuffer>* accumulate, T first, Args... args )
    {
        FormatAccumulator( accumulate, first );
        FormatAccumulator( accumulate, args... );
    }
    template<typename T>
    static void FormatAccumulator( DArray<StringBuffer>* accumlate, T last )
    {
        Allocator temp_alloc = ArenaAllocator::Create( &CoreContext::core_arena );
        StringBuffer str = ToString( last, temp_alloc );
        DArray<StringBuffer>::Add( accumlate, str );
    }


    static void ConcatInternal( StringBuffer buffer, size_t curr_index, StringView start )
    {
        for ( size_t i = 0; i < start.length; ++i )
        {
            buffer.buffer[curr_index + i] = start.buffer[i];
        }
    }

    template<typename ...Args>
    static void ConcatInternal( StringBuffer buffer, size_t curr_index, StringView start, Args... more )
    {
        for ( size_t i = 0; i < start.length; ++i )
        {
            buffer.buffer[curr_index + i] = start.buffer[i];
        }

        ConcatInternal( buffer, curr_index + start.length, more... );
    }

    static inline size_t GetLength( StringView str )
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
        size_t total_size = GetLength( start, args... );
        return Concat( alloc, total_size, start, args... );
    }

    template<typename ...Args>
    static size_t GetLength( StringView first, Args... more )
    {
        StringView args[] = { more... };

        const size_t count = sizeof...(more);

        size_t res = first.length;

        for ( size_t i = 0; i < count; ++i )
        {
            res += args[i].length;
        }

        return res;
    }

    template<typename ...Args>
    static StringBuffer Format( Allocator alloc, StringView str, Args... args )
    {
        size_t arena_offset = CoreContext::core_arena.offset;
        Allocator temp_alloc = ArenaAllocator::Create( &CoreContext::core_arena );

        size_t max_slots = sizeof( size_t ) * (str.length / 2);
        size_t* formatSlots = (size_t*) temp_alloc.alloc( temp_alloc, max_slots );
        size_t formatSlotCount = 0;

        for ( size_t i = 0; i < str.length - 1; ++i )
        {
            char first = str.buffer[i];
            char second = str.buffer[i + 1];

            if ( (first == '{') && (second == '}') )
            {
                // note: we increment i again to skit the second bracket since that comparison is redundant
                formatSlots[formatSlotCount++] = i++;
            }
        }

        const size_t argsCount = sizeof...(args);

        DArray<StringBuffer> params = {};
        DArray<StringBuffer>::Create( argsCount, &params, temp_alloc );

        FormatAccumulator( &params, args... );

        assert( params.size == formatSlotCount );

        size_t size = str.length - (formatSlotCount * 2);

        for ( size_t i = 0; i < params.size; ++i )
        {
            size += params.data[i].length;
        }

        StringBuffer res = StringBuffer::Create( size, alloc );

        // copy to the final string
        {
            size_t prev_res_i = 0;
            size_t prev_temp_i = 0;

            for ( size_t p = 0; p < formatSlotCount; ++p )
            {
                StringBuffer curr_param = params.data[p];
                size_t offset = formatSlots[p];

                // copy the previous "template" text
                size_t prev_size = offset - prev_temp_i;
                CoreContext::mem_copy( (void*) (str.buffer + prev_temp_i), (void*) (res.buffer + prev_res_i), prev_size );

                prev_temp_i += prev_size + 2;

                // copy the word into the slot
                CoreContext::mem_copy( (void*) curr_param.buffer, (void*) (res.buffer + prev_res_i + prev_size), curr_param.length );

                prev_res_i += prev_size + curr_param.length;
            }

            // copy the remaining part
            {
                size_t remaining_temp = formatSlots[formatSlotCount - 1] + 2;
                CoreContext::mem_copy( (void*) (str.buffer + remaining_temp), (void*) (res.buffer + prev_res_i), str.length - remaining_temp );
            }
        }

        CoreContext::core_arena.offset = arena_offset;

        return res;
    }


    static bool Compare( const StringView a, const StringView b )
    {
        if ( a.length != b.length )
            return false;

        bool keep_looping = true;

        for ( size_t i = 0; i < a.length && keep_looping; ++i )
        {
            keep_looping &= (a.buffer[i] == b.buffer[i]);
        }

        return true;
    }


    static bool Compare( const char* a, const char* b, size_t length )
    {
        size_t curr_len = 0;
        bool are_equal = true;
        while ( (curr_len < length) && (*a != '\0') && (are_equal = *(a++) == *(b++)) )
        {
            curr_len++;
        }

        return are_equal;
    }    
    
    static inline size_t GetCStrLength( const char* str )
    {
        size_t cnt = 0;

        while ( str[cnt] != '\0' )
        {
            ++cnt;
        }

        return cnt;
    }
    static size_t ReplaceChar( char* string, const char to_replace, const char replace_with )
    {
        size_t curr_index = 0;
        size_t occurence = 0;
        char curr_char;

        while ( (curr_char = string[curr_index]) != '\0' )
        {
            if ( curr_char == to_replace )
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
        size_t* and_indicies = (size_t*) temp_alloc.alloc( temp_alloc, length_needed );
        size_t and_count = 0;
        for ( size_t i = 0; i < len; ++i )
        {
            if ( Compare( (original.buffer + i), to_replace.buffer, old_char_len ) )
            {
                and_indicies[and_count++] = i;
            }
        }

        StringBuffer new_string = StringBuffer::Create( len + and_count, allocator );
        size_t old_string_start = 0;
        size_t new_string_start = 0;

        for ( size_t i = 0; i < and_count; ++i )
        {
            size_t index = and_indicies[i];
            size_t size_to_copy = index - old_string_start;

            CoreContext::mem_copy( (void*) (original.buffer + old_string_start), new_string.buffer + new_string_start, size_to_copy );

            old_string_start += size_to_copy + old_char_len;
            new_string_start += size_to_copy;

            CoreContext::mem_copy( (void*) (replace_with.buffer), new_string.buffer + new_string_start, size_to_copy );

            new_string_start += new_char_len;
        }

        CoreContext::mem_copy( (void*) (original.buffer + old_string_start), new_string.buffer + new_string_start, len - old_string_start );

        return new_string;
    }

    static StringBuffer GetFilename( StringView path, Allocator allocator )
    {
        _ASSERT( path.buffer != nullptr );

        size_t last_anti_slash = -1;
        size_t total_length = 0;
        char curr_char;

        while ( (total_length < path.length) && (curr_char = path.buffer[total_length++]) != '\0' )
        {
            if ( curr_char == '\\' )
            {
                last_anti_slash = total_length;
            }
        }

        return StringBuffer::Create( path.buffer, last_anti_slash + 1, total_length - 1, allocator );
    }


    static StringBuffer GetExtension( StringView path, Allocator alloc )
    {
        _ASSERT( path.buffer != nullptr );

        size_t last_dot = -1;
        size_t total_length = 0;
        char curr_char;

        while ( (total_length < path.length) && (curr_char = path.buffer[total_length++]) != '\0' )
        {
            if ( curr_char == '.' )
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

        while ( (total_length < path.length) && (curr_char = path.buffer[total_length++]) != '\0' )
        {
            if ( curr_char == '\\' )
            {
                last_anti_slash = total_length;
            }
        }
        return StringBuffer::Create( path.buffer, 0, last_anti_slash, alloc );
    }

};
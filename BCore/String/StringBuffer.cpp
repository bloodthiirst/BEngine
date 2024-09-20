#include <memory>
#include "../Context/CoreContext.h"
#include "../Allocators/Allocator.h"
#include "StringBuffer.h"
#include "StringView.h"

StringBuffer StringBuffer::Create( size_t length, Allocator allocator )
{
    StringBuffer res;

    res.alloc = allocator;
    res.length = length;
    res.buffer = (char*) ALLOC(allocator, res.length * sizeof( char ) );

    return res;
}

StringBuffer StringBuffer::Create( const char* str, size_t from, size_t to, Allocator allocator )
{
    StringBuffer res = {};

    res.alloc = allocator;
    res.length = to - from + 1;

    res.buffer = (char*) ALLOC( allocator, res.length * sizeof( char ) );

    CoreContext::mem_copy( (void*) (str + from), res.buffer, res.length );

    return res;
}


StringBuffer StringBuffer::Create( const char* str, Allocator allocator )
{
    StringBuffer res = {};

    res.alloc = allocator;
    res.length = strlen( str );
    res.buffer = (char*) ALLOC( allocator, res.length * sizeof( char ) );
    CoreContext::mem_copy( (void*) str, res.buffer, res.length );

    return res;
}

StringBuffer StringBuffer::Create( StringView str, Allocator allocator )
{
    StringBuffer res;

    res.alloc = allocator;
    res.length = str.length;
    res.buffer = (char*) ALLOC( allocator, res.length * sizeof( char ) );
    


    CoreContext::mem_copy( (void*) str.buffer, res.buffer, res.length );
    return res;
}

void StringBuffer::Destroy( StringBuffer* str )
{
    str->alloc.free( &str->alloc, str->buffer );
    str->buffer = nullptr;
    str->length = 0;
}

void StringBuffer::Resize( StringBuffer* str, size_t length )
{
    if ( str->length <= length )
        return;

    str->buffer = (char*) str->alloc.realloc( &str->alloc, str->buffer, length * sizeof( char ) );
    str->length = str->length;

}
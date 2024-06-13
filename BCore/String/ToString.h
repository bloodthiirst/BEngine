#pragma once
#include "../String/StringBuffer.h"
template <typename T>
static StringBuffer ToString( T value, Allocator alloc );

static StringBuffer ToString( int32_t value, Allocator alloc )
{
    size_t length = 0;

    int32_t value_abs = abs( value );

    int32_t has_sign_offset = 0;

    if ( value < 0 )
    {
        has_sign_offset = 1;
    }

    int32_t remainder = value_abs;
    
    do
    {
        length++;
        remainder /= 10;
    } 
    while ( remainder != 0 );

    StringBuffer str = StringBuffer::Create( length + has_sign_offset, alloc );

    if ( value < 0 )
    {
        str.buffer[0] = '-';
    }

    for ( int i = (int) length - 1; i > -1; --i )
    {
        int32_t digit = value_abs % 10;
        value_abs /= 10;

        char digit_char = (char) (digit + 48);
        str.buffer[i] = digit_char;
    }

    return str;
}

static StringBuffer ToString( const char* value, Allocator alloc )
{
    StringBuffer res = StringBuffer::Create( value, alloc );

    return res;
}

static StringBuffer ToString( char* value, Allocator alloc )
{
    StringBuffer res = StringBuffer::Create( value, alloc );

    return res;
}

static StringBuffer ToString( float value, Allocator alloc )
{
    char* buf = (char*) alloc.alloc( &alloc, 50);
    sprintf_s( buf, 50 , "%g", value );

    return StringBuffer::Create( buf, alloc );
}

static StringBuffer ToString( uint32_t value, Allocator alloc )
{
    size_t length = 0;

    uint32_t val = value;

    int32_t remainder = val;

    do
    {
        length++;
        remainder /= 10;
    }
    while ( remainder != 0 );

    StringBuffer str = StringBuffer::Create( length, alloc );

    for ( int i = (int) length - 1; i > -1; --i )
    {
        int32_t digit = value % 10;
        value /= 10;

        char digit_char = (char) (digit + 48);
        str.buffer[i] = digit_char;
    }

    return str;
}
#include <memory>
#include <string.h>
#include "StringView.h"
#include "StringBuffer.h"
#include "../Allocators/Allocator.h"

StringView StringView::Create(const char* str)
{
    return StringView(str);
}

char* StringView::ToCString(StringView str_view, Allocator alloc)
{
    char* c_str =  (char*) alloc.alloc(alloc, sizeof(char) * (str_view.length + 1));
    memcpy(c_str, str_view.buffer, str_view.length);
    c_str[str_view.length] = '\0';

    return c_str;
}

StringView::StringView( const char* c_str )
{
    buffer = c_str;
    length = strlen( c_str );
}
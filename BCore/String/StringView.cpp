#include "../Context/CoreContext.h"
#include "StringView.h"
#include "StringBuffer.h"
#include "StringUtils.h"
#include "../Allocators/Allocator.h"

StringView StringView::Create(const char* str)
{
    return StringView(str);
}

char* StringView::ToCString(StringView str_view, Allocator alloc)
{
    char* c_str =  (char*) alloc.alloc( &alloc, sizeof(char) * (str_view.length + 1));
    CoreContext::mem_copy((void*)str_view.buffer , c_str, str_view.length);
    c_str[str_view.length] = '\0';

    return c_str;
}
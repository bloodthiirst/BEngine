#include <assert.h>
#include <stdio.h>
#include "FileUtils.h"

StringBuffer FileUtils::ReadFile(const StringView file_path, Allocator alloc)
{
    FILE* file_handle;
    fopen_s(&file_handle , file_path.buffer, "r");

    assert(file_handle);
    
    size_t file_size = fseek(file_handle, 0L, SEEK_END) + 1;

    fseek(file_handle, 0L, SEEK_SET);
    StringBuffer file_data = StringBuffer::Create(file_size, alloc);

    fgets(file_data.buffer, (int) file_size, file_handle);
    fclose(file_handle);

    return file_data;
}

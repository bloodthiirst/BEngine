#pragma once
#include <String/StringView.h>
#include <String/StringBuffer.h>
#include <Allocators/Allocator.h>

class FileUtils
{

public:
	static StringBuffer ReadFile ( const StringView filePath , Allocator alloc );
};


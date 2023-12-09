#pragma once
#include <String/StringView.h>

struct ApplicationState
{
	bool isRunning;
    bool isFocused;
    StringView executablePath;
};


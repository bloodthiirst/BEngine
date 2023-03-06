#include "ConsoleLogger.h"
#include <windows.h>
#include <cstdio>
#include <iostream>


void ConsoleLogger::Log ( const char* message, va_list args )
{
	FlushConsoleInputBuffer ( hConsole );
	SetConsoleTextAttribute ( hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY );

	std::cout << "[Log]:::" << message << std::endl;

    SetConsoleTextAttribute ( hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
}

void ConsoleLogger::Info ( const char* message, va_list args )
{
	FlushConsoleInputBuffer ( hConsole );
	SetConsoleTextAttribute ( hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);

	std::cout << "[Info]:::" << message << std::endl;

    SetConsoleTextAttribute ( hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
}

void ConsoleLogger::Warning ( const char* message, va_list args )
{
	FlushConsoleInputBuffer ( hConsole );
	SetConsoleTextAttribute ( hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY );

	std::cout << "[Warning]:::" << message << std::endl;

    SetConsoleTextAttribute ( hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
}

void ConsoleLogger::NewLine ()
{
	std::cout << '\n';
}

void ConsoleLogger::Error ( const char* message, va_list args )
{
	FlushConsoleInputBuffer ( hConsole );
	SetConsoleTextAttribute ( hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY );

	std::cout << "[Error]:::" << message << std::endl;

    SetConsoleTextAttribute ( hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
}

void ConsoleLogger::Fatal ( const char* message, va_list args )
{
	FlushConsoleInputBuffer ( hConsole );
	SetConsoleTextAttribute ( hConsole, FOREGROUND_RED );

	std::cout << "[Fatal]:::" << message << std::endl;

    SetConsoleTextAttribute ( hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED );
}

ConsoleLogger::ConsoleLogger ()
{
	AllocConsole ();

	FILE* fpstdin = stdin;
	FILE* fpstdout = stdout;
	FILE* fpstderr = stderr;

	freopen_s ( &fpstdin, "CONIN$", "r", stdin );
	freopen_s ( &fpstdout, "CONOUT$", "w", stdout );
	freopen_s ( &fpstderr, "CONOUT$", "w", stderr );

	hConsole = GetStdHandle ( STD_OUTPUT_HANDLE );
}

ConsoleLogger::~ConsoleLogger ()
{
	FreeConsole ();
}

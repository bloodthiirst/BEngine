#pragma once
#include "../Base/ILogger.h"
#include <windows.h>
class ConsoleLogger :
	public ILogger
{
private :
	HANDLE hConsole;
public :

	ConsoleLogger ();

	void Log ( const char* message, va_list args );

	void Info ( const char* message, va_list args );

	void Warning ( const char* message, va_list args );

	void NewLine ();

	void Error ( const char* message, va_list args );

	void Fatal ( const char* message, va_list args );
	
	~ConsoleLogger ();
};


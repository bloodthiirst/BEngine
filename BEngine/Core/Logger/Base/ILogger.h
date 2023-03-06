#pragma once
#include <stdarg.h>

class ILogger
{
public :

	virtual void Log ( const char* message , va_list args ) = 0;

	virtual void Info ( const char* message, va_list args ) = 0;

	virtual void NewLine () = 0;

	virtual void Warning ( const char* message, va_list args ) = 0;

	virtual void Error ( const char* message, va_list args ) = 0;

	virtual void Fatal ( const char* message, va_list args ) = 0;
};


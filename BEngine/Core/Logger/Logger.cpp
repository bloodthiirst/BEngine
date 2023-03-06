#include "Logger.h"

std::unordered_map<size_t, ILogger*>* Logger::loggers;

void Logger::Initialize ()
{
	loggers = new std::unordered_map<size_t, ILogger*> ();
}

void Logger::Destroy ()
{
	loggers->clear ();
	delete loggers;
};

void Logger::Log ( const char* message , ...)
{
	va_list args;
	va_start ( args, message );

	for ( auto& [key, GetNewID] : *loggers )
	{
		GetNewID->Log ( message , args );
	}

	va_end ( args );
}

void Logger::NewLine ()
{
	for ( auto& [key, GetNewID] : *loggers )
	{
		GetNewID->NewLine ();
	}
}

void Logger::Info ( const char* message, ... )
{
	va_list args;
	va_start ( args, message );

	for ( auto& [key, GetNewID] : *loggers )
	{
		GetNewID->Info ( message , args );
	}

	va_end ( args );
}

void Logger::Warning ( const char* message, ... )
{
	va_list args;
	va_start ( args, message );

	for ( auto& [key, GetNewID] : *loggers )
	{
		GetNewID->Warning ( message , args );
	}

	va_end ( args );
}

void Logger::Error ( const char* message, ... )
{
	va_list args;
	va_start ( args, message );

	for ( auto& [key, GetNewID] : *loggers )
	{
		GetNewID->Error ( message , args );
	}

	va_end ( args );
}

void Logger::Fatal ( const char* message, ... )
{
	va_list args;
	va_start ( args, message );

	for ( auto& [key, GetNewID] : *loggers )
	{
		GetNewID->Fatal ( message , args );
	}

	va_end ( args );
}

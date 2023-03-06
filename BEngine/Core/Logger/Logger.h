#pragma once
#include <vector>
#include "Base/ILogger.h"
#include <typeinfo>
#include <unordered_map>
#include <cstdarg>

class Logger
{
private:
	static std::unordered_map<size_t, ILogger*>* loggers;

public:
	Logger () = delete;
	static void Initialize ();
	static void Destroy ();

public :
	template<typename Tlogger>
	static Tlogger* Add ()
	{
		const size_t id = typeid(Tlogger).hash_code ();

		auto findResult = loggers->find ( id );

		if ( findResult == loggers->end () )
		{
			Tlogger* logger = new Tlogger ();

			auto pair = std::make_pair(id , logger);
			loggers->insert ( pair );
			return logger;
		}

		return (Tlogger*) findResult->second;
	}

	template<class Tlogger>
	static bool Remove ()
	{
		const size_t id = typeid(Tlogger).hash_code();

		return loggers.erase ( id );
	}

public:
	static void Log ( const char* message , ... );
	static void NewLine ();
	static void Info (const char* message, ... );
	static void Warning (const char* message, ... );
	static void Error (const char* message, ... );
	static void Fatal (const char* message, ... );
};


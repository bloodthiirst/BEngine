#include "Logger.h"

void Logger::Initialize()
{
    loggers = std::unordered_map<size_t, ILogger>();
}

void Logger::Destroy()
{
    loggers.clear();
};
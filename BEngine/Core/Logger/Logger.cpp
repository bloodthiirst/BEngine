#include "Logger.h"

void Logger::Initialize()
{
    loggers = std::unordered_map<size_t, ILogger>();
}

void Logger::Destroy()
{
    loggers.clear();
};

BAPI void Logger::Log(StringView message, ...)
{
    va_list args;
    va_start(args, message);

    for (size_t i = 0; i < loggers.size(); ++i)
    {
        ILogger logger = loggers.at(i);
        logger.log(&logger, message);
    }

    va_end(args);
}

void Logger::NewLine(size_t repeat)
{
    for (auto& [key, logger] : loggers)
    {
        logger.new_line(&logger, repeat);
    }
}

void Logger::Info(StringView message, ...)
{
    va_list args;
    va_start(args, message);

    for (auto& [key, logger] : loggers)
    {
        logger.info(&logger, message);
    }

    va_end(args);
}

void Logger::Warning(StringView message, ...)
{
    va_list args;
    va_start(args, message);

    for (auto& [key, logger] : loggers)
    {
        logger.warning(&logger, message);
    }

    va_end(args);
}

void Logger::Error(StringView message, ...)
{
    va_list args;
    va_start(args, message);

    for (auto& [key, logger] : loggers)
    {
        logger.fatal(&logger, message);
    }

    va_end(args);
}

void Logger::Fatal(StringView message, ...)
{
    va_list args;
    va_start(args, message);

    for (auto& [key, logger] : loggers)
    {
        logger.fatal(&logger, message);
    }

    va_end(args);
}

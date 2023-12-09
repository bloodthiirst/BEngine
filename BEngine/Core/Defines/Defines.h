#pragma once

#ifdef _ENGINE_EXPORT
#define BAPI __declspec(dllexport)
#else
#define BAPI  __declspec(dllimport)
#endif

class Defines
{
public:
    static char const* engine_name;
};
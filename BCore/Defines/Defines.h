#pragma once

#ifdef _CORE_IMPORT_STATIC_LIB
    #define CORE_API 
#elif _CORE_EXPORT
    #define CORE_API __declspec(dllexport)
#else
    #define CORE_API  __declspec(dllimport)
#endif
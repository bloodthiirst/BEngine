#pragma once

#ifdef _CORE_EXPORT
    #define CORE_API __declspec(dllexport)
#else
    #define CORE_API  __declspec(dllimport)
#endif

#ifdef _DEBUG

    #define ALLOC_WITH_INFO(allocator , size , metadata) \
        allocator.alloc( &allocator, size ); \
        {\
        const char filename[] = __FILE__; \
        const int line = __LINE__;\
        AllocationData data = {};\
        data.line_number = line;\
        data.filepath = filename;\
        data.user_data = metadata;\
        if(allocator.callbacks.alloc_callback){\
            allocator.callbacks.alloc_callback(&allocator , size , data);\
        }\


    #define ALLOC_NO_INFO(allocator , size) \
        allocator.alloc( &allocator, size ); \
        {\
        const char filename[] = __FILE__; \
        const int line = __LINE__;\
        AllocationData data = {};\
        data.line_number = line;\
        data.filepath = filename;\
        data.user_data = nullptr;\
        if(allocator.callbacks.alloc_callback){\
            allocator.callbacks.alloc_callback(&allocator , size , data);\
        }\
        }\

    #define GET_MACRO(_1,_2,_3,NAME,...) NAME

    #define ALLOC(...) GET_MACRO(__VA_ARGS__,ALLOC_WITH_INFO, ALLOC_NO_INFO , )(__VA_ARGS__)

#else

    #define ALLOC(allocator , size) \
    {\
    allocator.alloc( &allocator, size ); \
    const char filename[] = __FILE__; \
    const int line = __LINE__;\
    }\

#endif
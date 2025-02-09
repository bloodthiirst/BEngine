#pragma once

#ifdef _ENGINE_EXPORT
#define BAPI __declspec(dllexport)
#else
#define BAPI  __declspec(dllimport)
#endif


#define VK_CHECK(X, RESULT)                     \
    VkResult RESULT = X;                        \
    if (RESULT != VK_SUCCESS)                   \
    {                                           \
        Global::logger.Error("Vulkan Error !"); \
        Global::logger.Error(#X);               \
    }                                           \

class Defines
{
public:
    static char const* engine_name;
};


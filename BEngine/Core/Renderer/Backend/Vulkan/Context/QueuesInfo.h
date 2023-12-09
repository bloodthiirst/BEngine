#pragma once
#include <vulkan/vulkan.h>

struct QueuesInfo
{
    uint32_t presentQueueFamilyIndex;
    uint32_t graphicsQueueIndex;
    uint32_t computeQueueFamilyIndex;
    uint32_t transferQueueIndex;

    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkQueue computeQueue;
    VkQueue transferQueue;
};
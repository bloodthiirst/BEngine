#pragma once
#include <vulkan/vulkan.h>
#include "QueuesInfo.h"
#include "SwapchainSupportInfo.h"
#include "../CommandBuffer/CommandPoolsInfo.h"



struct PhysicalDeviceInfo
{
    QueuesInfo queues_info;
    CommandPoolsInfo command_pools_info;
    SwapchainSupportInfo swapchainSupportInfo;
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkFormat depthFormat;

    bool FindMemoryIndex (uint32_t typeFilter, uint32_t propertyFlags, uint32_t* outMemeoryIndex );

};
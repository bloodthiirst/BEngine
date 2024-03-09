#pragma once
#include <vulkan/vulkan.h>
#include "QueuesInfo.h"
#include "SwapchainSupportInfo.h"
#include "../CommandBuffer/CommandPoolsInfo.h"



struct PhysicalDeviceInfo
{
public:
    QueuesInfo queuesInfo;
    CommandPoolsInfo commandPoolsInfo;
    SwapchainSupportInfo swapchainSupportInfo;
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkFormat depthFormat;

public:
    bool FindMemoryIndex (uint32_t typeFilter, uint32_t propertyFlags, uint32_t* outMemeoryIndex );

};
#pragma once
#include <vulkan/vulkan.h>
#include "QueuesInfo.h"
#include "SwapchainSupportInfo.h"
#include "CommandPoolsInfo.h"



struct PhysicalDeviceInfo
{
public:
    QueuesInfo queuesInfo;
    CommandPoolsInfo commandPoolsInfo;
    SwapchainSupportInfo swapchainSupportInfo;
public:
    VkPhysicalDevice handle;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures physicalDeviceFeatures;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkFormat depthFormat;

public:
    bool FindMemoryIndex (uint32_t typeFilter, uint32_t propertyFlags, uint32_t* outMemeoryIndex );

};
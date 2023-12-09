#pragma once
#include <vulkan/vulkan.h>
#include <Containers/DArray.h>

struct SwapchainSupportInfo
{
public:
    VkSurfaceCapabilitiesKHR capabilities;
    DArray<VkSurfaceFormatKHR> surfaceFormats;
    DArray<VkPresentModeKHR> presentModes;
};
#pragma once
#include <vulkan/vulkan.h>
#include <vector>

struct SwapchainSupportInfo
{
public:
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};
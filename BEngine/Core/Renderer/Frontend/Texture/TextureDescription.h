#pragma once
#include <vulkan/vulkan.h>

struct TextureDescriptor
{
public:
    VkImageType imageType;
    uint32_t width;
    uint32_t height;
    VkFormat format;
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkMemoryPropertyFlags memoryFlags;
    bool createView;
    VkImageAspectFlags viewAspectFlags;
};
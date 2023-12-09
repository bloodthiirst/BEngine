#pragma once
#include <vulkan/vulkan.h>

struct VulkanContext;
struct TextureDescriptor;

struct TextureDescriptor
{
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

struct Texture
{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    uint32_t width;
    uint32_t height;

    static void Destroy ( VulkanContext* context, Texture* texture );
    static void Create ( VulkanContext* context, TextureDescriptor descriptor, Texture* texture );
};
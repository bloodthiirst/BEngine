#pragma once
#include <vulkan/vulkan.h>

struct VulkanContext;
struct TextureDescriptor;

class Texture
{
public:
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    uint32_t width;
    uint32_t height;

public:
    static void Destroy ( VulkanContext* context, Texture* texture );
    static void Create ( VulkanContext* context, TextureDescriptor descriptor, Texture* texture );
};
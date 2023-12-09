#pragma once
#include "Texture.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"

inline void CreateView ( VulkanContext* context, TextureDescriptor descriptor, Texture* texture )
{
    VkImageViewCreateInfo createViewInfo = {};
    createViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createViewInfo.format = descriptor.format;
    createViewInfo.image = texture->handle;
    createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    createViewInfo.subresourceRange.aspectMask = descriptor.viewAspectFlags;
    createViewInfo.subresourceRange.baseArrayLayer = 0;
    createViewInfo.subresourceRange.levelCount = 1;
    createViewInfo.subresourceRange.layerCount = 1;
    createViewInfo.subresourceRange.baseMipLevel = 0;

    vkCreateImageView ( context->logicalDeviceInfo.handle, &createViewInfo, context->allocator, &texture->view );
}

void Texture::Destroy ( VulkanContext* context, Texture* texture )
{
    if ( texture->view )
    {
        vkDestroyImageView ( context->logicalDeviceInfo.handle, texture->view, context->allocator );
        texture->view = nullptr;
    }

    if ( texture->memory )
    {
        vkFreeMemory ( context->logicalDeviceInfo.handle, texture->memory, context->allocator );
        texture->memory = nullptr;
    }

    if ( texture->handle )
    {
        vkDestroyImage ( context->logicalDeviceInfo.handle, texture->handle, context->allocator );
        texture->handle = nullptr;
    }
}

void Texture::Create( VulkanContext* context, TextureDescriptor descriptor, Texture* texture )
{
    texture->width = descriptor.width;
    texture->height = descriptor.height;

    VkImageCreateInfo createImageInfo = {};
    createImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createImageInfo.imageType = descriptor.imageType;
    createImageInfo.extent.width = descriptor.width;
    createImageInfo.extent.height = descriptor.height;
    createImageInfo.extent.depth = 1;
    createImageInfo.mipLevels = 4;
    createImageInfo.arrayLayers = 1;
    createImageInfo.format = descriptor.format;
    createImageInfo.tiling = descriptor.tiling;
    createImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createImageInfo.usage = descriptor.usage;
    createImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    createImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage ( context->logicalDeviceInfo.handle, &createImageInfo, context->allocator, &texture->handle );

    VkMemoryRequirements memoryReqs = {};

    vkGetImageMemoryRequirements ( context->logicalDeviceInfo.handle, texture->handle, &memoryReqs );

    uint32_t memoryTypeIndex = 0;
    context->physicalDeviceInfo.FindMemoryIndex (memoryReqs.memoryTypeBits, descriptor.memoryFlags, &memoryTypeIndex );
     

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryReqs.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    // allocate
    VkDeviceMemory gpuImageMemory = {};
    vkAllocateMemory ( context->logicalDeviceInfo.handle, &allocateInfo, context->allocator, &gpuImageMemory );

    // bind
    vkBindImageMemory ( context->logicalDeviceInfo.handle, texture->handle, gpuImageMemory, 0 ); // todo : make the momery offset able to be non 0

    texture->memory = gpuImageMemory;

    if ( descriptor.createView )
    {
        CreateView ( context, descriptor, texture );
    }
}


#pragma once
#include "Texture.h"
#include "../Context/VulkanContext.h"

inline void CreateView(VulkanContext *context, TextureDescriptor descriptor, Texture *texture)
{
    VkImageViewCreateInfo createViewInfo = {};
    createViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createViewInfo.format = descriptor.format;
    createViewInfo.image = texture->handle;
    createViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    createViewInfo.subresourceRange.aspectMask = descriptor.view_aspect_flags;
    createViewInfo.subresourceRange.baseArrayLayer = 0;
    createViewInfo.subresourceRange.levelCount = 1;
    createViewInfo.subresourceRange.layerCount = 1;
    createViewInfo.subresourceRange.baseMipLevel = 0;

    vkCreateImageView(context->logical_device_info.handle, &createViewInfo, context->allocator, &texture->view);
}

void Texture::Destroy(Texture *texture)
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    if (texture->view)
    {
        vkDestroyImageView(context->logical_device_info.handle, texture->view, context->allocator);
        texture->view = nullptr;
    }

    if (texture->memory)
    {
        vkFreeMemory(context->logical_device_info.handle, texture->memory, context->allocator);
        texture->memory = nullptr;
    }

    if (texture->handle)
    {
        vkDestroyImage(context->logical_device_info.handle, texture->handle, context->allocator);
        texture->handle = nullptr;
    }
}

void Texture::Create(TextureDescriptor descriptor, Texture *texture)
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    texture->width = descriptor.width;
    texture->height = descriptor.height;

    VkImageCreateInfo createImageInfo = {};
    createImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

    createImageInfo.imageType = descriptor.image_type;
    createImageInfo.extent.width = descriptor.width;
    createImageInfo.extent.height = descriptor.height;
    createImageInfo.extent.depth = 1;

    createImageInfo.mipLevels = 4;
    createImageInfo.arrayLayers = 1;
    createImageInfo.format = descriptor.format;
    createImageInfo.tiling = descriptor.tiling;
    createImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createImageInfo.usage = descriptor.usage;
    createImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;         // the number of samples per pixel , this can be changed based on if the device supports multisampling
    createImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // implies that the image can only be accessed by a single queue family at a time

    vkCreateImage(context->logical_device_info.handle, &createImageInfo, context->allocator, &texture->handle);

    VkMemoryRequirements memoryReqs = {};

    vkGetImageMemoryRequirements(context->logical_device_info.handle, texture->handle, &memoryReqs);

    uint32_t memoryTypeIndex = 0;
    context->physical_device_info.FindMemoryIndex(memoryReqs.memoryTypeBits, descriptor.memory_flags, &memoryTypeIndex);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memoryReqs.size;
    allocateInfo.memoryTypeIndex = memoryTypeIndex;

    // allocate
    VkDeviceMemory gpuImageMemory = {};
    vkAllocateMemory(context->logical_device_info.handle, &allocateInfo, context->allocator, &gpuImageMemory);

    // bind
    vkBindImageMemory(context->logical_device_info.handle, texture->handle, gpuImageMemory, 0); // todo : make the momery offset able to be non 0

    texture->memory = gpuImageMemory;

    if (descriptor.create_view)
    {
        CreateView(context, descriptor, texture);
    }
}

void Texture::TransitionLayout(Texture *texture, CommandBuffer cmd, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    // a barrier is basically a sync point that indicates that whatever used the memory beforehand use the old content/memory
    // and whatever comes after use the new content/memory
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = context->physical_device_info.queues_info.graphicsQueueIndex;
    barrier.dstQueueFamilyIndex = context->physical_device_info.queues_info.graphicsQueueIndex;
    barrier.image = texture->handle;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    VkPipelineStageFlags source_stage = {0};
    VkPipelineStageFlags dest_stage = {0};

    // if we don't care about the old layout - and we want the new layout to be optimal for the driver
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        // we don't care about where we are at the pipeline
        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

        // used for copying
        dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    // if we want to change from load image to a shader readonly layout
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        // transition from a transfer destination layout to a shader readonly layout
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        // says that we want to go from a coying stage to ...
        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        // ... a shader reading state
        dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        Global::logger.Log("Unsupported layout transition");
        return;
    }

    vkCmdPipelineBarrier(cmd.handle, source_stage, dest_stage, 0, 0, 0, 0, 0, 1, &barrier);
}

void Texture::CopyFromBuffer(VkBuffer from_buffer, Texture *to_texture, CommandBuffer copy_cmd)
{

    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    VkBufferImageCopy buffer_to_image_copy = {};
    buffer_to_image_copy.bufferOffset = 0;
    buffer_to_image_copy.bufferRowLength = 0;
    buffer_to_image_copy.bufferImageHeight = 0;

    buffer_to_image_copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    buffer_to_image_copy.imageSubresource.layerCount = 1;
    buffer_to_image_copy.imageSubresource.baseArrayLayer = 0;
    buffer_to_image_copy.imageSubresource.mipLevel = 0;

    buffer_to_image_copy.imageExtent.width = to_texture->width;
    buffer_to_image_copy.imageExtent.height = to_texture->height;
    buffer_to_image_copy.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(copy_cmd.handle, from_buffer, to_texture->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_to_image_copy);
}

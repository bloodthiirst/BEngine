#pragma once
#include <vulkan/vulkan.h>
#include "../../../Global/Global.h"
#include "../../Backend/Vulkan/Context/CommandBuffer.h"

struct VulkanContext;
struct TextureDescriptor;

struct TextureDescriptor
{
    /// <summary>
    /// Defines the image type (1D , 2D , 3D)
    /// </summary>
    VkImageType image_type;
    uint32_t width;
    uint32_t height;
    VkFormat format;

    /// <summary>
    /// <para>(Defined as swizzling in DirectX) It is the addressing layout of texels within an image. This is currently opaque and it is not defined when you access it using the CPU </para>
    /// <para>-OPTIMAL : specifies optimal tiling (texels are laid out in an implementation-dependent arrangement, for more optimal memory access)</para>
    /// <para>-LINEAR : specifies linear tiling (texels are laid out in memory in row-major order, possibly with some padding on each row)</para>
    /// </summary>
    VkImageTiling tiling;

    /// <summary>
    /// <para>Used mainly to define the use-case that can be done with the image like :</para>
    /// <para>- Being a color buffer for a Framebuffer</para>
    /// <para>- Being a source/destination in transform command</para>
    /// <para>- Being sampled from in a shader's descriptor set</para>
    /// </summary>
    VkImageUsageFlags usage;

    VkMemoryPropertyFlags memory_flags;

    bool create_view;
    VkImageAspectFlags view_aspect_flags;
};

struct Texture
{
    VkImage handle;
    VkImageView view;
    VkDeviceMemory memory;
    uint32_t width;
    uint32_t height;

    static void Destroy( VulkanContext* context, Texture* texture );
    static void Create( VulkanContext* context, TextureDescriptor descriptor, Texture* texture );

    static void TransitionLayout( VulkanContext* context, Texture* texture, CommandBuffer cmd, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout )
    {
        // a barrier is basically a sync point that indicates that whatever used the memory beforehand use the old content/memory
        // and whatever comes after use the new content/memory
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.image = texture->handle;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

        // if we don't care about the old layout - and we want the new layout to be optimal for the driver
        if ( old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
             new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            // we don't care about where we are at the pipeline
            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

            // used for copying
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }

        // if we want to change from load image to a shader readonly layout
        else if ( old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && 
                  new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
        {
            // transition from a transfer destination layout to a shader readonly layout
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            // says the we want to go from a coying stage to ...
            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;

            // ... a shader reading state
            source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            Global::logger.Log( "Unsupported layout transition" );
            return;
        }

        vkCmdPipelineBarrier( cmd.handle, source_stage, dest_stage, 0, 0, 0, 0, 0, 1, &barrier );
    }

    static void Copy( VulkanContext* context, VkBuffer from_buffer, Texture* to_texture, CommandBuffer copy_cmd )
    {
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

        vkCmdCopyBufferToImage( copy_cmd.handle, from_buffer, to_texture->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_to_image_copy );
    }

};

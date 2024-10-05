#pragma once
#include <vulkan/vulkan.h>
#include "../../Defines/Defines.h"
#include "../../Global/Global.h"
#include "../CommandBuffer/CommandBuffer.h"

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

    uint32_t mipmaps_level;

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

struct BAPI Texture
{
    VkImage handle;
    VkImageView view;
    VkDeviceMemory memory;
    uint32_t width;
    uint32_t height;

    static void Destroy(Texture* texture );
    static void Create(TextureDescriptor descriptor, Texture* texture );
    static void TransitionLayout(Texture* texture, CommandBuffer cmd, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout );
    static void CopyFromBuffer(VkBuffer from_buffer, Texture* to_texture, CommandBuffer copy_cmd );

};

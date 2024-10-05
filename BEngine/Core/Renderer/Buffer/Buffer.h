#pragma once
#include <vulkan/vulkan.h>
#include "../Fence/Fence.h"
#include "../../Defines/Defines.h"

struct VulkanContext;
struct Memory;

struct BufferDescriptor
{
    uint32_t size;
    VkBufferUsageFlagBits usage;
    VkSharingMode sharing_mode;
    VkMemoryPropertyFlagBits memoryPropertyFlags;
};

struct BAPI Buffer
{
    VkBuffer handle;
    BufferDescriptor descriptor;
    VkDeviceMemory memory;
    bool isLocked;
    uint32_t memoryIndex;

    static bool Create (BufferDescriptor descriptor , bool bindOnCreate ,Buffer* out_buffer );
    static bool Destroy (Buffer* out_buffer );
    static bool Load (uint32_t buffer_offset, uint32_t size , void* in_data , VkMemoryMapFlags flags , Buffer* inout_buffer );
    
    /// <summary>
    /// Takes the content of the buffer and maps it to a block of memory (outDataPtr in out case)
    /// </summary>
    /// <param name="context"></param>
    /// <param name="offset"></param>
    /// <param name="size"></param>
    /// <param name="flags"></param>
    /// <param name="inBuffer"></param>
    /// <param name="outDataPtr"></param>
    /// <returns></returns>
    static bool Lock (uint32_t offset, uint32_t size , VkMemoryMapFlags flags , Buffer* in_buffer, void** data );
    
    /// <summary>
    /// Does the opposite of Lock , unmaps the memory and automatically cleans the memory passed to us by Lock (outDataPtr)
    /// </summary>
    /// <param name="context"></param>
    /// <param name="inBuffer"></param>
    /// <returns></returns>
    static bool Unlock (Buffer* in_buffer );
    static bool Copy (VkCommandPool pool, Fence fence, VkQueue queue, Buffer* src, uint32_t srcOffset, Buffer* dst, uint32_t dstOffset, uint32_t size );
    static bool Resize (uint32_t newSize, VkQueue queue, VkCommandPool pool, Buffer* in_buffer );
    static bool Bind (uint32_t offset , Buffer* in_buffer );
};
#pragma once
#include <vulkan/vulkan.h>
#include "../Fence/Fence.h"

struct VulkanContext;
struct Memory;

struct BufferDescriptor
{
    uint32_t size;
    VkBufferUsageFlagBits usage;
    VkMemoryPropertyFlagBits memoryPropertyFlags;
};

class Buffer
{
public:
    VkBuffer handle;
    BufferDescriptor descriptor;
    VkDeviceMemory memory;
    bool isLocked;
    uint32_t memoryIndex;

public:
    static bool Create ( VulkanContext* context, BufferDescriptor descriptor , bool bindOnCreate ,Buffer* outBuffer );
    static bool Destroy ( VulkanContext* context, Buffer* outBuffer );
    static bool Load ( VulkanContext* context, uint32_t offset, uint32_t size , void* inDataPtr , uint32_t flags , Buffer* inBuffer );
    
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
    static bool Lock ( VulkanContext* context,uint32_t offset, uint32_t size , uint32_t flags , Buffer* inBuffer, void** data );
    
    /// <summary>
    /// Does the opposite of Lock , unmaps the memory and automatically cleans the memory passed to us by Lock (outDataPtr)
    /// </summary>
    /// <param name="context"></param>
    /// <param name="inBuffer"></param>
    /// <returns></returns>
    static bool Unlock ( VulkanContext* context, Buffer* inBuffer );
    static bool Copy ( VulkanContext* context, VkCommandPool pool, Fence fence, VkQueue queue, Buffer src, uint32_t srcOffset, Buffer dst, uint32_t dstOffset, uint32_t size );
    static bool Copy ( VulkanContext* context, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer src, uint32_t srcOffset, VkBuffer dst, uint32_t dstOffset, uint32_t size );
    static bool Copy ( VulkanContext* context, VkCommandPool pool, Fence fence, VkQueue queue, Buffer* src, uint32_t srcOffset, Buffer* dst, uint32_t dstOffset, uint32_t size );
    static bool Resize ( VulkanContext* context, uint32_t newSize, VkQueue queue, VkCommandPool pool, Buffer* inBuffer );
    static bool Bind ( VulkanContext* context, uint32_t offset , Buffer* inBuffer );
};
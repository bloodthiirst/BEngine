#include "Buffer.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"
#include "../../../Logger/Logger.h"
#include "../../../Global/Global.h"
#include "../../../Platform/Base/Memory.h"

bool Buffer::Create( VulkanContext* context, BufferDescriptor descriptor, bool bind_on_create, Buffer* out_buffer )
{
    out_buffer->descriptor = descriptor;

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = descriptor.size;
    createInfo.usage = descriptor.usage;
    // this means that this buffer will be used with only one queue
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult res = vkCreateBuffer( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &out_buffer->handle );

    if (res != VK_SUCCESS)
    {
        Global::logger.Error( "Couldn't create buffer" );
        return false;
    }
    VkMemoryRequirements memReqs = {};
    vkGetBufferMemoryRequirements( context->logicalDeviceInfo.handle, out_buffer->handle, &memReqs );


    if (!context->physicalDeviceInfo.FindMemoryIndex( memReqs.memoryTypeBits, descriptor.memoryPropertyFlags, &out_buffer->memoryIndex ))
    {
        Global::logger.Error( "Couldn't find appropriate memory index for buffer" );
        return false;
    }

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize = memReqs.size;
    allocateInfo.memoryTypeIndex = out_buffer->memoryIndex;


    res = vkAllocateMemory( context->logicalDeviceInfo.handle, &allocateInfo, context->allocator, &out_buffer->memory );

    if (res != VK_SUCCESS)
    {
        Global::logger.Error( "Couldn't allocate memory" );
        return false;
    }

    if (bind_on_create)
    {
        if (!Buffer::Bind( context, 0, out_buffer ))
        {
            Global::logger.Error( "Couldn't bind memory" );
            return false;
        }
    }

    return true;
}

bool Buffer::Destroy( VulkanContext* context, Buffer* out_buffer )
{
    vkFreeMemory( context->logicalDeviceInfo.handle, out_buffer->memory, context->allocator );

    vkDestroyBuffer( context->logicalDeviceInfo.handle, out_buffer->handle, context->allocator );

    *out_buffer = {};

    return true;
}

bool Buffer::Load( VulkanContext* context, uint32_t offset, uint32_t size, void* in_data_ptr, uint32_t flags, Buffer* in_buffer )
{
    void* tmpDataPtr = nullptr;
    Buffer::Lock( context, offset, size, flags, in_buffer, &tmpDataPtr );

    Global::platform.memory.mem_copy( in_data_ptr, tmpDataPtr, size );

    Buffer::Unlock( context, in_buffer );

    return true;
}

bool Buffer::Lock( VulkanContext* context, uint32_t offset, uint32_t size, uint32_t flags, Buffer* in_buffer, void** data )
{
    VkResult res = vkMapMemory( context->logicalDeviceInfo.handle, in_buffer->memory, offset, size, flags, data );
    in_buffer->isLocked = true;

    return true;
}

bool Buffer::Unlock( VulkanContext* context, Buffer* in_buffer )
{
    vkUnmapMemory( context->logicalDeviceInfo.handle, in_buffer->memory );
    in_buffer->isLocked = false;
    return true;
}

bool Buffer::Copy( VulkanContext* context, VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer src, uint32_t srcOffset, VkBuffer dst, uint32_t dstOffset, uint32_t size )
{
    vkQueueWaitIdle( queue );

    CommandBuffer cmd = {};
    CommandBuffer::SingleUseAllocateBegin( context, pool, &cmd );

    VkBufferCopy bufferCpy = {};
    bufferCpy.srcOffset = srcOffset;
    bufferCpy.dstOffset = dstOffset;
    bufferCpy.size = size;

    vkCmdCopyBuffer( cmd.handle, src, dst, 1, &bufferCpy );

    CommandBuffer::SingleUseEndSubmit( context, pool, &cmd, queue );

    return true;
}

bool Buffer::Copy( VulkanContext* context, VkCommandPool pool, Fence fence, VkQueue queue, Buffer* src, uint32_t srcOffset, Buffer* dst, uint32_t dstOffset, uint32_t size )
{
    return Copy( context, pool, fence.handle, queue, src->handle, srcOffset, dst->handle, dstOffset, size );
}

bool Buffer::Resize( VulkanContext* context, uint32_t newSize, VkQueue queue, VkCommandPool pool, Buffer* inBuffer )
{
    // buffers are immutable , so they can't really be "resize"
    // so resizing the buffer is pretty much 
    // - recreating a new memory with new size
    // - copying old data to new memory
    // - bind new memory to buffer
    // - delete old memory
    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = newSize;
    createInfo.usage = inBuffer->descriptor.usage;
    // this means that this buffer will be used with only one queue
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer newBuffer = {};
    VkResult res = vkCreateBuffer( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &newBuffer );

    if (res != VK_SUCCESS)
    {
        Global::logger.Error( "Couldn't create buffer" );
        return false;
    }
    VkMemoryRequirements memReqs = {};
    vkGetBufferMemoryRequirements( context->logicalDeviceInfo.handle, newBuffer, &memReqs );


    if (!context->physicalDeviceInfo.FindMemoryIndex( memReqs.memoryTypeBits, inBuffer->descriptor.memoryPropertyFlags, &inBuffer->memoryIndex ))
    {
        Global::logger.Error( "Couldn't find appropriate memory index for buffer" );
        return false;
    }

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.allocationSize = memReqs.size;
    allocateInfo.memoryTypeIndex = inBuffer->memoryIndex;

    VkDeviceMemory newMemory = {};
    res = vkAllocateMemory( context->logicalDeviceInfo.handle, &allocateInfo, context->allocator, &newMemory );

    vkBindBufferMemory( context->logicalDeviceInfo.handle, newBuffer, newMemory, 0 );

    Copy( context, pool, nullptr, queue, inBuffer->handle, 0, newBuffer, 0, inBuffer->descriptor.size );

    vkDeviceWaitIdle( context->logicalDeviceInfo.handle );

    vkFreeMemory( context->logicalDeviceInfo.handle, inBuffer->memory, context->allocator );

    vkDestroyBuffer( context->logicalDeviceInfo.handle, inBuffer->handle, context->allocator );

    inBuffer->descriptor.size = newSize;
    inBuffer->memory = newMemory;
    inBuffer->handle = newBuffer;

    return true;
}

bool Buffer::Bind( VulkanContext* context, uint32_t offset, Buffer* in_buffer )
{
    VkResult res = vkBindBufferMemory( context->logicalDeviceInfo.handle, in_buffer->handle, in_buffer->memory, offset );

    return res == VK_SUCCESS;
}
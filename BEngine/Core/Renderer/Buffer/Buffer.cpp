#include "Buffer.h"
#include "../Context/VulkanContext.h"
#include "../../Logger/Logger.h"
#include "../../Global/Global.h"
#include "../../Platform/Base/Memory.h"

bool Buffer::Create(BufferDescriptor descriptor, bool bind_on_create, Buffer* out_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;
    
    out_buffer->descriptor = descriptor;

    VkBufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = descriptor.size;
    createInfo.usage = descriptor.usage;
    // this means that this buffer will be used with only one queue
    createInfo.sharingMode = descriptor.sharing_mode;

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
        if (!Buffer::Bind(0, out_buffer ))
        {
            Global::logger.Error( "Couldn't bind memory" );
            return false;
        }
    }

    return true;
}

bool Buffer::Destroy(Buffer* out_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    vkFreeMemory( context->logicalDeviceInfo.handle, out_buffer->memory, context->allocator );

    vkDestroyBuffer( context->logicalDeviceInfo.handle, out_buffer->handle, context->allocator );

    *out_buffer = {};

    return true;
}

bool Buffer::Load(uint32_t offset, uint32_t size, void* in_data_ptr, VkMemoryMapFlags flags, Buffer* in_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    void* mapped_memory_ptr = nullptr;
    
    Buffer::Lock(offset, size, flags, in_buffer, &mapped_memory_ptr );

    Global::platform.memory.mem_copy( in_data_ptr, mapped_memory_ptr, size );

    Buffer::Unlock(in_buffer );

    return true;
}

bool Buffer::Lock(uint32_t offset, uint32_t size, VkMemoryMapFlags flags, Buffer* in_buffer, void** data )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    VkResult res = vkMapMemory( context->logicalDeviceInfo.handle, in_buffer->memory, offset, size, flags, data );
    in_buffer->isLocked = true;

    return true;
}

bool Buffer::Unlock(Buffer* in_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    vkUnmapMemory( context->logicalDeviceInfo.handle, in_buffer->memory );
    in_buffer->isLocked = false;
    return true;
}

bool Buffer::Copy(VkCommandPool pool, Fence fence, VkQueue queue, Buffer* src, uint32_t srcOffset, Buffer* dst, uint32_t dstOffset, uint32_t size )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    vkQueueWaitIdle( queue );

    CommandBuffer cmd = {};
    CommandBuffer::SingleUseAllocateBegin(pool, &cmd );

    VkBufferCopy bufferCpy = {};
    bufferCpy.srcOffset = srcOffset;
    bufferCpy.dstOffset = dstOffset;
    bufferCpy.size = size;

    vkCmdCopyBuffer( cmd.handle, src->handle, dst->handle, 1, &bufferCpy );

    CommandBuffer::SingleUseEndSubmit(pool, &cmd, queue );

    return true;
}

bool Buffer::Resize(uint32_t new_size, VkQueue queue, VkCommandPool pool, Buffer* in_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    // buffers are immutable , so they can't really be "resize"
    // so resizing the buffer is pretty much 
    // - recreating a new memory with new size
    // - copying old data to new memory
    // - bind new memory to buffer
    // - delete old memory
    BufferDescriptor new_buffer_desc = in_buffer->descriptor;
    new_buffer_desc.size = new_size;
    
    Buffer new_buffer = {};
    Buffer::Create(new_buffer_desc , true , &new_buffer);

    Copy(pool, {}, queue, in_buffer, 0, &new_buffer, 0, in_buffer->descriptor.size );

    vkDeviceWaitIdle( context->logicalDeviceInfo.handle );

    vkFreeMemory( context->logicalDeviceInfo.handle, in_buffer->memory, context->allocator );

    vkDestroyBuffer( context->logicalDeviceInfo.handle, in_buffer->handle, context->allocator );

    in_buffer->descriptor.size = new_size;
    in_buffer->memory = new_buffer.memory;
    in_buffer->handle = new_buffer.handle;

    return true;
}

bool Buffer::Bind(uint32_t offset, Buffer* in_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    VK_CHECK(vkBindBufferMemory( context->logicalDeviceInfo.handle, in_buffer->handle, in_buffer->memory, offset ) , res);

    return res == VK_SUCCESS;
}
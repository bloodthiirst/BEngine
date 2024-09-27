#include "CommandBuffer.h"
#include "../Context/VulkanContext.h"

void CommandBuffer::Begin ( bool isSingleUse, bool isRenderpassContinue, bool isSimultanious )
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    if ( isSingleUse )
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }

    if ( isRenderpassContinue )
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    }

    if ( isSimultanious )
    {
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    }

    vkBeginCommandBuffer ( this->handle, &beginInfo );
    this->state = CommandBufferState::Recording;
}

void CommandBuffer::UpdateSubmitted ()
{
    this->state = CommandBufferState::Submitted;
}

void CommandBuffer::Reset ()
{
    this->state = CommandBufferState::Ready;
}

void CommandBuffer::End ()
{
    vkEndCommandBuffer ( handle );
    this->state = CommandBufferState::RecordingEnded;
}


void CommandBuffer::Allocate(VkCommandPool pool, bool isPrimary, CommandBuffer* out_command_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    *out_command_buffer = {};
    
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = pool;

    // a secondary is command buffer that can used within another command buffer and can't be used on it's own
    allocateInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = nullptr;

    out_command_buffer->state = CommandBufferState::NoAllocated;

    vkAllocateCommandBuffers ( context->logical_device_info.handle, &allocateInfo, &out_command_buffer->handle );

    out_command_buffer->state = CommandBufferState::Ready;
}

void CommandBuffer::Free (VkCommandPool pool, CommandBuffer* outCommandBuffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    vkFreeCommandBuffers ( context->logical_device_info.handle, pool, 1, &outCommandBuffer->handle );
    outCommandBuffer->handle = 0;
    outCommandBuffer->state = CommandBufferState::NoAllocated;
}

void CommandBuffer::SingleUseAllocateBegin (VkCommandPool pool, CommandBuffer* out_command_buffer )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;

    out_command_buffer->Allocate (pool, true, out_command_buffer );
    out_command_buffer->Begin ( true, false, false );
}

void CommandBuffer::SingleUseEndSubmit (VkCommandPool pool, CommandBuffer* out_command_buffer , VkQueue queue )
{
    VulkanContext *context = (VulkanContext *)Global::backend_renderer.user_data;
    
    out_command_buffer->End ();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &out_command_buffer->handle;
 
    vkQueueSubmit ( queue, 1, &submitInfo, nullptr );

    vkQueueWaitIdle ( queue );

    CommandBuffer::Free ( pool, out_command_buffer );
}
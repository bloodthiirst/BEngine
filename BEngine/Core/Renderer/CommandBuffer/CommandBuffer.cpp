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


void CommandBuffer::Allocate( VulkanContext* context, VkCommandPool pool, bool isPrimary, CommandBuffer* outCommandBuffer )
{
    *outCommandBuffer = {};
    
    VkCommandBufferAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = pool;

    // a secondary is command buffer that can used within another command buffer and can't be used on it's own
    allocateInfo.level = isPrimary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext = nullptr;

    outCommandBuffer->state = CommandBufferState::NoAllocated;

    vkAllocateCommandBuffers ( context->logicalDeviceInfo.handle, &allocateInfo, &outCommandBuffer->handle );

    outCommandBuffer->state = CommandBufferState::Ready;
}

void CommandBuffer::Free ( VulkanContext* context, VkCommandPool pool, CommandBuffer* outCommandBuffer )
{
    vkFreeCommandBuffers ( context->logicalDeviceInfo.handle, pool, 1, &outCommandBuffer->handle );
    outCommandBuffer->handle = 0;
    outCommandBuffer->state = CommandBufferState::NoAllocated;
}

void CommandBuffer::SingleUseAllocateBegin ( VulkanContext* context, VkCommandPool pool, CommandBuffer* outCommandBuffer )
{
    CommandBuffer::Allocate ( context, pool, true, outCommandBuffer );
    outCommandBuffer->Begin ( true, false, false );
}

void CommandBuffer::SingleUseEndSubmit ( VulkanContext* context, VkCommandPool pool, CommandBuffer* outCommandBuffer , VkQueue queue )
{
    outCommandBuffer->End ();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &outCommandBuffer->handle;
 
    vkQueueSubmit ( queue, 1, &submitInfo, nullptr );

    vkQueueWaitIdle ( queue );

    CommandBuffer::Free ( context, pool, outCommandBuffer );
}
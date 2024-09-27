#include "FrameBuffer.h"
#include "../Context/VulkanContext.h"

void FrameBuffer::Create ( VulkanContext* context, Renderpass* renderpass, Vector2Int dimensions, DArray<VkImageView> attatchments, FrameBuffer* outFramebuffer )
{
    outFramebuffer->attatchments = attatchments;
    outFramebuffer->renderpass = renderpass;

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderpass->handle;
    createInfo.attachmentCount = (uint32_t) outFramebuffer->attatchments.size;
    createInfo.pAttachments = outFramebuffer->attatchments.data;
    createInfo.width = dimensions.x;
    createInfo.height = dimensions.y;
    createInfo.layers = 1;

    vkCreateFramebuffer ( context->logical_device_info.handle, &createInfo, context->allocator, &outFramebuffer->handle );
}


void FrameBuffer::Destroy ( VulkanContext * context, FrameBuffer * outFramebuffer )
{
    vkDestroyFramebuffer ( context->logical_device_info.handle, outFramebuffer->handle, context->allocator );
    DArray< VkImageView>::Destroy(&outFramebuffer->attatchments);
    outFramebuffer->handle = nullptr;
    outFramebuffer->renderpass = nullptr;
}

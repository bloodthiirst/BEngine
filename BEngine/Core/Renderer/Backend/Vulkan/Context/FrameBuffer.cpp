#include "FrameBuffer.h"
#include "VulkanContext.h"

void FrameBuffer::Create ( VulkanContext* context, Renderpass* renderpass, Vector2Int dimensions, std::vector<VkImageView>* attatchments, FrameBuffer* outFramebuffer )
{
    outFramebuffer->attatchments = *attatchments;
    outFramebuffer->renderpass = renderpass;

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderpass->handle;
    createInfo.attachmentCount = outFramebuffer->attatchments.size ();
    createInfo.pAttachments = outFramebuffer->attatchments.data ();
    createInfo.width = dimensions.x;
    createInfo.height = dimensions.y;
    createInfo.layers = 1;

    vkCreateFramebuffer ( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &outFramebuffer->handle );
}


void FrameBuffer::Destroy ( VulkanContext * context, FrameBuffer * outFramebuffer )
{
    vkDestroyFramebuffer ( context->logicalDeviceInfo.handle, outFramebuffer->handle, context->allocator );
    outFramebuffer->attatchments.clear ();
    outFramebuffer->handle = nullptr;
    outFramebuffer->renderpass = nullptr;
}

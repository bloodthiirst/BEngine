#include "FrameBuffer.h"
#include "../Context/VulkanContext.h"

void FrameBuffer::Create ( VulkanContext* context, Renderpass* in_renderpass, Vector2Int dimensions, DArray<VkImageView> attatchments, FrameBuffer* out_framebuffer )
{
    out_framebuffer->attatchments = attatchments;
    out_framebuffer->renderpass = in_renderpass;

    VkFramebufferCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = in_renderpass->handle;
    createInfo.attachmentCount = (uint32_t) out_framebuffer->attatchments.size;
    createInfo.pAttachments = out_framebuffer->attatchments.data;
    createInfo.width = dimensions.x;
    createInfo.height = dimensions.y;
    createInfo.layers = 1;

    vkCreateFramebuffer ( context->logical_device_info.handle, &createInfo, context->allocator, &out_framebuffer->handle );
}

void FrameBuffer::Destroy ( VulkanContext * context, FrameBuffer * inout_framebuffer )
{
    vkDestroyFramebuffer ( context->logical_device_info.handle, inout_framebuffer->handle, context->allocator );
    DArray<VkImageView>::Destroy(&inout_framebuffer->attatchments);
    *inout_framebuffer = {};
}

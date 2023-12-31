#include "Renderpass.h"
#include "VulkanContext.h"
#include "FrameBuffer.h"

void Renderpass::Begin ( VulkanContext* context , CommandBuffer* cmdBuffer , FrameBuffer* frameBuffer )
{
        VkRenderPassBeginInfo begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        begin_info.renderPass = handle;
        begin_info.framebuffer = frameBuffer->handle;
        begin_info.renderArea.offset.x = (int32_t) area.x;
        begin_info.renderArea.offset.y = (int32_t) area.y;
        begin_info.renderArea.extent.width = (int32_t) area.width;
        begin_info.renderArea.extent.height = (int32_t) area.height;

        VkClearValue clear_values[2];

        clear_values[0].color.float32[0] = clearColor.r;
        clear_values[0].color.float32[1] = clearColor.g;
        clear_values[0].color.float32[2] = clearColor.b;
        clear_values[0].color.float32[3] = clearColor.a;
        clear_values[1].depthStencil.depth = depth;
        clear_values[1].depthStencil.stencil = stencil;

        begin_info.clearValueCount = 2;
        begin_info.pClearValues = clear_values;

        vkCmdBeginRenderPass ( cmdBuffer->handle, &begin_info, VK_SUBPASS_CONTENTS_INLINE );

        cmdBuffer->state = CommandBufferState::InRenderpass;
}

void Renderpass::End ( VulkanContext* context, CommandBuffer* commandBuffer )
{
    vkCmdEndRenderPass ( commandBuffer->handle );
    commandBuffer->state = CommandBufferState::Recording;
}

bool Renderpass::Create ( VulkanContext* context, Rect rect, Color color, float depth, uint32_t stencil, Renderpass* renderpass )
{
    // main subpass
    VkSubpassDescription subpassDesc = {};
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    
    Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

    DArray<VkAttachmentDescription> attachementDescs;
    DArray<VkAttachmentDescription>::Create( 2, &attachementDescs, heap_alloc);

    VkAttachmentDescription colorAttachmentDesc = {};
    VkAttachmentReference colorReference = {};

    // color attachement
    {
        colorAttachmentDesc.format = context->swapchain_info.surfaceFormat.format;
        colorAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        // loadOp defines what to do to the texture when we load it
        // we can clear OnLoad , keep the previous content , or don't care
        // color
        colorAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

        // storeOp defines what to do to the texture when we store it
        // we can "store it" , (which seems redundant in terms of naming) but means keep the content of this texture to be used by things to come after
        // or don't care , which means that the content wont be used after we're done using it
        colorAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // we're not usin stencil for now , so don't care
        // stencil
        colorAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colorAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
        colorAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // what we're gonna transition to after the renderpass , it's a layout format in memory
        colorAttachmentDesc.flags = 0;

        // specifies the
        colorReference.attachment = 0; // attachement index
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout

        DArray< VkAttachmentDescription>::Add( &attachementDescs , colorAttachmentDesc);
    }

    VkAttachmentDescription depthAttachmentDesc = {};
    VkAttachmentReference depthReference = {};

    // depth attachement
    {
        depthAttachmentDesc.format = context->physicalDeviceInfo.depthFormat;
        depthAttachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

        // loadOp define what to do to the texture when we load it
        // we can clear OnLoad , keep the previous content , or don't care
        // color
        depthAttachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

        // storeOp definew what to do to the textre when we store it
        // we can "store i" , which seems redundant but means keep the content of this texture to be used by things to come after
        // or don't care , which means that the content wont be used after we're done using it
        depthAttachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // we're not usin stencil for now , so don't care
        // stencil
        depthAttachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        depthAttachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // since this is a brand new "create" , we're not creating this renderpass using an existing image from a previous renderpass , so no "expected" image layout
        depthAttachmentDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // what we're gonna transition to after the renderpass , it's a layout format in memory
        depthAttachmentDesc.flags = 0;

        // specifies the 
        depthReference.attachment = 1; // attachement index
        depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // layout

        DArray< VkAttachmentDescription>::Add( &attachementDescs, depthAttachmentDesc );
    }

    // todo : we could have other attacheent (input , resolve , preserve)

    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorReference;
    subpassDesc.pDepthStencilAttachment = &depthReference;

    // input from a shader
    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;

    // use for multip sampling color attachment (???? look it up)
    subpassDesc.pResolveAttachments = 0;

    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;

    // render pass dependencies , can be multiple dependencies
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // render pass
    VkRenderPassCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.attachmentCount = (uint32_t) attachementDescs.size;
    createInfo.pAttachments = attachementDescs.data;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpassDesc;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;
    createInfo.flags = 0;
    createInfo.pNext = 0;

    VkRenderPass vkRenderpass = {};
    vkCreateRenderPass ( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &vkRenderpass );

    renderpass->handle = vkRenderpass;
    renderpass->area = rect;
    renderpass->clearColor = color;
    renderpass->depth = depth;
    renderpass->stencil = stencil;

    return true;

}

void Renderpass::Destroy ( VulkanContext* context, Renderpass* renderpass )
{
    vkDestroyRenderPass ( context->logicalDeviceInfo.handle, renderpass->handle, context->allocator );
}
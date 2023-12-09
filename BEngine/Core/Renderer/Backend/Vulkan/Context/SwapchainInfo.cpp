#include <Maths/Maths.h>
#include "../../../../Global/Global.h"
#include "SwapchainInfo.h"
#include "VulkanContext.h"
#include "../../../Frontend/Texture/Texture.h"
#include "../../../../Logger/Logger.h"
#include "SwapchainSupportInfo.h"
#include "Fence.h"


bool QueryDepthBufferFormat( VulkanContext* context, VkFormat* outDepthFormat )
{
    VkFormat depthFormats[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    uint32_t flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for (uint32_t i = 0; i < 3; ++i)
    {
        VkFormatProperties formatProps = {};
        vkGetPhysicalDeviceFormatProperties( context->physicalDeviceInfo.handle, depthFormats[i], &formatProps );

        VkFormat curr = depthFormats[i];

        if ((formatProps.linearTilingFeatures & flags) == flags)
        {
            *outDepthFormat = curr;
            return true;
        }

        if ((formatProps.optimalTilingFeatures & flags) == flags)
        {
            *outDepthFormat = curr;
            return true;
        }
    }

    return false;

}

void SwapchainInfo::QuerySwapchainSupport( VkPhysicalDevice handle, VkSurfaceKHR surface, SwapchainSupportInfo* outSwapchainInfo )
{
    // capabilities
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( handle, surface, &capabilities );

    Allocator heap_alloc = HeapAllocator::Create();
    // surface formats
    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( handle, surface, &formatsCount, nullptr );
    DArray<VkSurfaceFormatKHR> formats;
    DArray<VkSurfaceFormatKHR>::Create( formatsCount, &formats, heap_alloc );

    vkGetPhysicalDeviceSurfaceFormatsKHR( handle, surface, &formatsCount, formats.data );

    // present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR( handle, surface, &presentModeCount, nullptr );
    DArray<VkPresentModeKHR> presentModes;
    DArray<VkPresentModeKHR>::Create( presentModeCount, &presentModes, heap_alloc );
    vkGetPhysicalDeviceSurfacePresentModesKHR( handle, surface, &presentModeCount, presentModes.data );

    // todo : here we add a check for device extensions

    outSwapchainInfo->capabilities = capabilities;
    outSwapchainInfo->presentModes = presentModes;
    outSwapchainInfo->surfaceFormats = formats;
}

bool SwapchainInfo::Destroy( VulkanContext* context, SwapchainInfo* outSwapchain )
{
    Texture::Destroy( context, &outSwapchain->depthAttachement );

    for (uint32_t i = 0; i < outSwapchain->imageViews.size; ++i)
    {
        vkDestroyImageView( context->logicalDeviceInfo.handle, outSwapchain->imageViews.data[i], context->allocator );
    }


    DArray<VkImageView>::Clear( &context->swapchain_info.imageViews );

    vkDestroySwapchainKHR( context->logicalDeviceInfo.handle, outSwapchain->handle, context->allocator );

    return true;
}

/// <summary>
/// <para>Free the command buffers</para>
/// <para>Destroy the frame buffers</para>
/// <para>Destroy semaphores and fence</para>
/// </summary>
/// <param name="context"></param>
/// <param name="outSwapchain"></param>
void SwapchainInfo::Clear( VulkanContext* context, SwapchainInfo* outSwapchain )
{
    for (uint32_t i = 0; i < outSwapchain->graphics_cmd_buffers_per_image.size; ++i)
    {
        CommandBuffer* curr = &outSwapchain->graphics_cmd_buffers_per_image.data[i];

        CommandBuffer::Free( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, curr );
    }

    DArray<CommandBuffer>::Clear( &outSwapchain->graphics_cmd_buffers_per_image );

    for (uint32_t i = 0; i < outSwapchain->frameBuffers.size; ++i)
    {
        FrameBuffer* curr = &outSwapchain->frameBuffers.data[i];

        FrameBuffer::Destroy( context, curr );
    }

    DArray<FrameBuffer>::Clear( &outSwapchain->frameBuffers );

    for (uint32_t i = 0; i < outSwapchain->in_flight_fence_per_image.size; ++i)
    {
        outSwapchain->in_flight_fence_per_image.data[i] = nullptr;
    }

    DArray<Fence*>::Clear( &outSwapchain->in_flight_fence_per_image );

    for (uint32_t i = 0; i < context->swapchain_info.imagesCount; ++i)
    {
        if (outSwapchain->image_presentation_complete_semaphores.data[i])
        {
            vkDestroySemaphore( context->logicalDeviceInfo.handle, outSwapchain->image_presentation_complete_semaphores.data[i], context->allocator );
            outSwapchain->image_presentation_complete_semaphores.data[i] = nullptr;
        }

        if (outSwapchain->finishedRenderingSemaphore.data[i])
        {
            vkDestroySemaphore( context->logicalDeviceInfo.handle, outSwapchain->finishedRenderingSemaphore.data[i], context->allocator );
            outSwapchain->finishedRenderingSemaphore.data[i] = nullptr;
        }

        Fence::Destroy( context, &outSwapchain->cmd_buffer_done_execution_per_frame.data[i] );
    }

    DArray<VkSemaphore>::Clear( &outSwapchain->image_presentation_complete_semaphores );
    DArray<VkSemaphore>::Clear( &outSwapchain->finishedRenderingSemaphore );
    DArray<Fence>::Clear( &outSwapchain->cmd_buffer_done_execution_per_frame );
}

bool SwapchainInfo::Create( VulkanContext* context, SwapchainCreateDescription descrption, SwapchainInfo* outSwapchain )
{
    // query the swapchain support
    QuerySwapchainSupport( context->physicalDeviceInfo.handle, context->surface, &context->physicalDeviceInfo.swapchainSupportInfo );

    VkExtent2D swapchainExtent = { descrption.width ,descrption.height };
    outSwapchain->maxFramesInFlight = descrption.imagesCount - 1;

    // we select the image format that the swapchain's image will use
    // first we try to get format => R8G8B8A8 and colorSpace =>  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // if we don't find it , then we default to the first available format given by the physical device
    {
        bool found = false;

        for (uint32_t i = 0; i < context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats.size; ++i)
        {
            VkSurfaceFormatKHR* curr = &context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats.data[i];

            if (curr->format == VK_FORMAT_R8G8B8A8_UNORM && curr->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                outSwapchain->surfaceFormat = *curr;
                found = true;
                break;
            }
        }

        if (!found)
        {
            outSwapchain->surfaceFormat = context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats.data[0];
        }
    }


    // we select the swapchain's present mode
    // vulkan specifices that all GPU must support FIFO so we start with it as the default mode
    // we query the physical devices modes and try to select MAILBOX if it's supported
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    {
        for (int i = 0; i < context->physicalDeviceInfo.swapchainSupportInfo.presentModes.size; ++i)
        {
            VkPresentModeKHR mode = context->physicalDeviceInfo.swapchainSupportInfo.presentModes.data[i];

            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                presentMode = mode;
                break;
            }
        }
    }

    swapchainExtent = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.currentExtent;

    VkExtent2D gpuMin = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.minImageExtent;
    VkExtent2D gpuMax = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.maxImageExtent;

    swapchainExtent.width = Maths::Clamp( swapchainExtent.width, gpuMin.width, gpuMax.width );
    swapchainExtent.height = Maths::Clamp( swapchainExtent.height, gpuMin.height, gpuMax.height );

    // minImage count is usually 2 , so +1 gives 3
    uint32_t imagesCount = descrption.imagesCount;

    imagesCount = Maths::Clamp
    (
        imagesCount,
        context->physicalDeviceInfo.swapchainSupportInfo.capabilities.minImageCount,
        context->physicalDeviceInfo.swapchainSupportInfo.capabilities.maxImageCount
    );

    // create swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.minImageCount = imagesCount;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageFormat = context->swapchain_info.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = context->swapchain_info.surfaceFormat.colorSpace;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.surface = context->surface;
    swapchainCreateInfo.oldSwapchain = nullptr;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // this deal with the case where the device can be in portait or horizontal mode
    swapchainCreateInfo.preTransform = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.currentTransform;
    uint32_t queueFamilyIndicies[] =
    {
        context->physicalDeviceInfo.queuesInfo.presentQueueFamilyIndex,
        context->physicalDeviceInfo.queuesInfo.graphicsQueueIndex,
    };

    if (context->physicalDeviceInfo.queuesInfo.presentQueueFamilyIndex != context->physicalDeviceInfo.queuesInfo.graphicsQueueIndex)
    {
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndicies;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }
    else
    {
        swapchainCreateInfo.queueFamilyIndexCount = 1;
        swapchainCreateInfo.pQueueFamilyIndices = &context->physicalDeviceInfo.queuesInfo.presentQueueFamilyIndex;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult createSwapchainResult = vkCreateSwapchainKHR( context->logicalDeviceInfo.handle, &swapchainCreateInfo, context->allocator, &outSwapchain->handle );

    context->current_frame = 0;
    outSwapchain->imagesCount = 0;

    // here we take the image count returned by vulkan even though we already specified a wanted count
    // we do this since the number returned can be less than the count we requested (for whatever reason) 
    VkResult getImagesResult = vkGetSwapchainImagesKHR( context->logicalDeviceInfo.handle, outSwapchain->handle, &outSwapchain->imagesCount, nullptr );

    Allocator alloc = Global::alloc_toolbox.heap_allocator;
    DArray<VkImage>::Create( imagesCount, &outSwapchain->images, alloc );
    outSwapchain->images.size = imagesCount;

    DArray<VkImageView>::Create( imagesCount, &outSwapchain->imageViews,  alloc );
    outSwapchain->imageViews.size = imagesCount;

    DArray<CommandBuffer>::Create( imagesCount, &outSwapchain->graphics_cmd_buffers_per_image,  alloc );
    Global::platform.memory.mem_set( outSwapchain->graphics_cmd_buffers_per_image.data  , 0 , sizeof(CommandBuffer) * imagesCount);
    outSwapchain->graphics_cmd_buffers_per_image.size = imagesCount;

    VkResult result = vkGetSwapchainImagesKHR( context->logicalDeviceInfo.handle, outSwapchain->handle, &outSwapchain->imagesCount, outSwapchain->images.data );

    // create imageViews for swapchain images
    for (uint32_t i = 0; i < outSwapchain->imagesCount; ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = outSwapchain->surfaceFormat.format;
        imageViewCreateInfo.image = outSwapchain->images.data[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;

        VkResult imageViewResult = vkCreateImageView( context->logicalDeviceInfo.handle, &imageViewCreateInfo, context->allocator, &outSwapchain->imageViews.data[i] );
    }

    // depth buffer format
    {
        if (!QueryDepthBufferFormat( context, &context->physicalDeviceInfo.depthFormat ))
        {
            context->physicalDeviceInfo.depthFormat = VK_FORMAT_UNDEFINED;
            Global::logger.Fatal( "Couldn't find depth format" );
            return false;
        }

        // create depth buffer image
        TextureDescriptor descriptor = {};
        descriptor.format = context->physicalDeviceInfo.depthFormat;
        descriptor.width = swapchainExtent.width;
        descriptor.height = swapchainExtent.height;
        descriptor.imageType = VK_IMAGE_TYPE_2D;
        descriptor.tiling = VK_IMAGE_TILING_OPTIMAL;
        descriptor.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        descriptor.memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        descriptor.createView = true;
        descriptor.viewAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;

        Texture depthTexture = {};
        Texture::Create( context, descriptor, &outSwapchain->depthAttachement );
    }

    // we create a command buffer for each swap chain image
    {
        // free the command buffers if any are allocated and reallocate
        for (uint32_t i = 0; i < outSwapchain->graphics_cmd_buffers_per_image.size; ++i)
        {
            CommandBuffer* curr = &outSwapchain->graphics_cmd_buffers_per_image.data[i];

            if (curr->handle != VK_NULL_HANDLE)
            {
                CommandBuffer::Free( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, curr );
            }

            CommandBuffer::Allocate( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, true, curr );
        }
    }


    // create sync objects
    {
        uint32_t imagesCount = context->swapchain_info.imagesCount;
        Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

        DArray<VkSemaphore>::Create( imagesCount, &outSwapchain->image_presentation_complete_semaphores, heap_alloc );
        outSwapchain->image_presentation_complete_semaphores.size = imagesCount;
        
        DArray<VkSemaphore>::Create( context->swapchain_info.imagesCount, &outSwapchain->finishedRenderingSemaphore, heap_alloc );
        outSwapchain->finishedRenderingSemaphore.size = imagesCount;
        
        DArray<Fence>::Create( context->swapchain_info.imagesCount, &outSwapchain->cmd_buffer_done_execution_per_frame, heap_alloc );
        outSwapchain->cmd_buffer_done_execution_per_frame.size = imagesCount;

        for (uint32_t i = 0; i < imagesCount; ++i)
        {
            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            // create semaphore for image available
            vkCreateSemaphore( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &outSwapchain->image_presentation_complete_semaphores.data[i] );

            // create semaphore for queue complete
            vkCreateSemaphore( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &outSwapchain->finishedRenderingSemaphore.data[i] );

            // the fences are initialized to true so that tha app doesn't block waiting for the first frame
            Fence::Create( context, true, &outSwapchain->cmd_buffer_done_execution_per_frame.data[i] );
        }

        DArray<Fence*>::Create( context->swapchain_info.imagesCount, &outSwapchain->in_flight_fence_per_image, heap_alloc );
        outSwapchain->in_flight_fence_per_image.size = imagesCount;
    }



    return true;
}

bool SwapchainInfo::CreateFrameBuffers( VulkanContext* context )
{
    // create frame buffers
    {
        Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

        DArray<FrameBuffer>::Create( imagesCount , &frameBuffers , heap_alloc );
        frameBuffers.size = imagesCount;

        for (uint32_t i = 0; i < imagesCount; ++i)
        {
            DArray<VkImageView> attachments;
            DArray<VkImageView>::Create( 2, &attachments, heap_alloc );
            attachments.size = 2;

            attachments.data[0] = imageViews.data[i];
            attachments.data[1] = depthAttachement.view;

            FrameBuffer frameBuffer = {};
            FrameBuffer::Create( context, &context->renderPass, context->frameBufferSize, attachments, &frameBuffers.data[i] );
        }
    }

    return true;
}

bool SwapchainInfo::Recreate( VulkanContext* context, SwapchainCreateDescription descrption, SwapchainInfo* outSwapchain )
{
    if (context->recreateSwapchain)
    {
        return false;
    }

    if (descrption.height == 0 || descrption.width == 0)
    {
        return false;
    }

    context->recreateSwapchain = true;

    vkDeviceWaitIdle( context->logicalDeviceInfo.handle );

    // recreate
    {
        Clear( context, outSwapchain );
        Destroy( context, outSwapchain );
        Create( context, descrption, outSwapchain );
        outSwapchain->CreateFrameBuffers( context );
    }

    context->frameBufferSizeLastGeneration = context->frameBufferSizeCurrentGeneration;

    context->renderPass.area.x = 0;
    context->renderPass.area.y = 0;
    context->renderPass.area.width = (float)context->frameBufferSize.x;
    context->renderPass.area.height = (float)context->frameBufferSize.y;

    context->recreateSwapchain = false;

    return true;
}


bool SwapchainInfo::AcquireNextImageIndex( VulkanContext* context, uint32_t timeout_ms, VkSemaphore semaphore, VkFence fence, uint32_t* out_next_image_index )
{
    VkResult result = vkAcquireNextImageKHR(
        context->logicalDeviceInfo.handle,
        this->handle,
        timeout_ms,
        semaphore,
        fence, out_next_image_index );


    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        SwapchainInfo::Destroy( context, this );

        SwapchainCreateDescription desc = {};
        desc.width = context->frameBufferSize.x;
        desc.height = context->frameBufferSize.y;

        SwapchainInfo::Create( context, desc, this );
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        Global::logger.Fatal( "Failed to acquire next image index" );
        return false;
    }

    return true;
}

bool SwapchainInfo::Present( VulkanContext* context, VkSemaphore render_complete_semaphore, uint32_t* in_present_image_index )
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &render_complete_semaphore;
    presentInfo.pSwapchains = &this->handle;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices = in_present_image_index;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR( context->physicalDeviceInfo.queuesInfo.presentQueue, &presentInfo );

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        SwapchainInfo::Destroy( context, &context->swapchain_info );

        SwapchainCreateDescription desc = {};
        desc.width = context->frameBufferSize.x;
        desc.height = context->frameBufferSize.y;

        SwapchainInfo::Create( context, desc, &context->swapchain_info );
        return false;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        Global::logger.Fatal( "Failed to acquire swapchain" );
        return false;
    }

    return true;
}

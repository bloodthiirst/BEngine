#include "SwapchainInfo.h"
#include "VulkanContext.h"
#include "../../../Frontend/Texture/TextureDescription.h"
#include "../../../../Maths/Maths.h"
#include "../../../../Logger/Logger.h"
#include "SwapchainSupportInfo.h"
#include "Fence.h"


bool QueryDepthBufferFormat ( VulkanContext* context , VkFormat* outDepthFormat )
{
    VkFormat depthFormats[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    uint32_t flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for ( uint32_t i = 0; i < 3; ++i )
    {
        VkFormatProperties formatProps = {};
        vkGetPhysicalDeviceFormatProperties ( context->physicalDeviceInfo.handle, depthFormats[i], &formatProps );

        VkFormat curr = depthFormats[i];

        if ( (formatProps.linearTilingFeatures & flags) == flags )
        {
            *outDepthFormat = curr;
            return true;
        }

        if ( (formatProps.optimalTilingFeatures & flags) == flags )
        {
            *outDepthFormat = curr;
            return true;
        }
    }

    return false;

}

void SwapchainInfo::QuerySwapchainSupport ( VkPhysicalDevice handle, VkSurfaceKHR surface, SwapchainSupportInfo* outSwapchainInfo )
{
    // capabilities
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( handle, surface, &capabilities );

    // surface formats
    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR ( handle, surface, &formatsCount, nullptr );
    std::vector<VkSurfaceFormatKHR> formats = std::vector<VkSurfaceFormatKHR> ( formatsCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR ( handle, surface, &formatsCount, formats.data () );

    // present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR ( handle, surface, &presentModeCount, nullptr );
    std::vector<VkPresentModeKHR> presentModes = std::vector<VkPresentModeKHR> ( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR ( handle, surface, &presentModeCount, presentModes.data () );

    // todo : here we add a check for device extensions

    outSwapchainInfo->capabilities = capabilities;
    outSwapchainInfo->presentModes = presentModes;
    outSwapchainInfo->surfaceFormats = formats;
}

bool SwapchainInfo::Destroy ( VulkanContext* context, SwapchainInfo* outSwapchain )
{
    Texture::Destroy ( context, &outSwapchain->depthAttachement );

    for ( uint32_t i = 0; i < outSwapchain->imageViews.size (); ++i )
    {
        vkDestroyImageView ( context->logicalDeviceInfo.handle, outSwapchain->imageViews[i], context->allocator );
    }

    context->swapchainInfo.imageViews.clear ();

    vkDestroySwapchainKHR ( context->logicalDeviceInfo.handle, outSwapchain->handle, context->allocator );

    return true;
}

/// <summary>
/// <para>Free the command buffers</para>
/// <para>Destroy the frame buffers</para>
/// <para>Destroy semaphores and fence</para>
/// </summary>
/// <param name="context"></param>
/// <param name="outSwapchain"></param>
void SwapchainInfo::Clear ( VulkanContext* context, SwapchainInfo* outSwapchain )
{
    for ( uint32_t i = 0; i < outSwapchain->graphicssCommandBuffers.size (); ++i )
    {
        CommandBuffer* curr = &outSwapchain->graphicssCommandBuffers[i];

        CommandBuffer::Free ( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, curr );
    }

    outSwapchain->graphicssCommandBuffers.clear ();

    for ( uint32_t i = 0; i < outSwapchain->frameBuffers.size (); ++i )
    {
        FrameBuffer* curr = &outSwapchain->frameBuffers[i];

        FrameBuffer::Destroy ( context, curr );
    }

    outSwapchain->frameBuffers.clear ();

    for ( uint32_t i = 0; i < outSwapchain->fencePtrAssociatedPerImageIndex.size (); ++i )
    {
        outSwapchain->fencePtrAssociatedPerImageIndex[i] = nullptr;
    }

    outSwapchain->fencePtrAssociatedPerImageIndex.clear ();

    for ( uint32_t i = 0; i < context->swapchainInfo.imagesCount; ++i )
    {
        if ( outSwapchain->imageAvailableSemaphore[i] )
        {
            vkDestroySemaphore ( context->logicalDeviceInfo.handle, outSwapchain->imageAvailableSemaphore[i], context->allocator );
            outSwapchain->imageAvailableSemaphore[i] = nullptr;
        }

        if ( outSwapchain->finishedRenderingSemaphore[i] )
        {
            vkDestroySemaphore ( context->logicalDeviceInfo.handle, outSwapchain->finishedRenderingSemaphore[i], context->allocator );
            outSwapchain->finishedRenderingSemaphore[i] = nullptr;
        }

        Fence::Destroy ( context, &outSwapchain->cmdBufferSumitFencePerFrameIndex[i] );
    }

    outSwapchain->imageAvailableSemaphore.clear ();
    outSwapchain->finishedRenderingSemaphore.clear ();
    outSwapchain->cmdBufferSumitFencePerFrameIndex.clear ();
}

bool SwapchainInfo::Create ( VulkanContext* context, SwapchainCreateDescription descrption, SwapchainInfo* outSwapchain )
{
    // query the swapchain support
    QuerySwapchainSupport ( context->physicalDeviceInfo.handle, context->surface, &context->physicalDeviceInfo.swapchainSupportInfo );

    VkExtent2D swapchainExtent = { descrption.width ,descrption.height };
    outSwapchain->maxFramesInFlight = descrption.imagesCount - 1;

    // we select the image format that the swapchain's image will use
    // first we try to get format => R8G8B8A8 and colorSpace =>  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // if we don't find it , then we default to the first available format given by the physical device
    {
        bool found = false;

        for ( uint32_t i = 0; i < context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats.size (); ++i )
        {
            VkSurfaceFormatKHR* curr = &context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats[i];

            if ( curr->format == VK_FORMAT_R8G8B8A8_UNORM && curr->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
            {
                outSwapchain->surfaceFormat = *curr;
                found = true;
                break;
            }
        }

        if ( !found )
        {
            outSwapchain->surfaceFormat = context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats[0];
        }
    }


    // we select the swapchain's present mode
    // vulkan specifices that all GPU must support FIFO so we start with it as the default mode
    // we query the physical devices modes and try to select MAILBOX if it's supported
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    {
        for ( int i = 0; i < context->physicalDeviceInfo.swapchainSupportInfo.presentModes.size (); ++i )
        {
            VkPresentModeKHR mode = context->physicalDeviceInfo.swapchainSupportInfo.presentModes[i];

            if ( mode == VK_PRESENT_MODE_MAILBOX_KHR )
            {
                presentMode = mode;
                break;
            }
        }
    }

    swapchainExtent = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.currentExtent;

    VkExtent2D gpuMin = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.minImageExtent;
    VkExtent2D gpuMax = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.maxImageExtent;

    swapchainExtent.width = Maths::Clamp ( swapchainExtent.width, gpuMin.width, gpuMax.width );
    swapchainExtent.height = Maths::Clamp ( swapchainExtent.height, gpuMin.height, gpuMax.height );

    // minImage count is usual 2 , so +1 gives 3
    uint32_t imagesCount =  descrption.imagesCount;

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
    swapchainCreateInfo.imageFormat = context->swapchainInfo.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = context->swapchainInfo.surfaceFormat.colorSpace;
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

    if ( context->physicalDeviceInfo.queuesInfo.presentQueueFamilyIndex != context->physicalDeviceInfo.queuesInfo.graphicsQueueIndex )
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

    VkResult createSwapchainResult = vkCreateSwapchainKHR ( context->logicalDeviceInfo.handle, &swapchainCreateInfo, context->allocator, &outSwapchain->handle );

    context->currentFrame = 0;
    outSwapchain->imagesCount = 0;

    // here we take the image count given by vulkan since it might be clameed compared to the count we requested
    VkResult getImagesResult = vkGetSwapchainImagesKHR ( context->logicalDeviceInfo.handle, outSwapchain->handle, &outSwapchain->imagesCount, nullptr );

    outSwapchain->images.resize ( outSwapchain->imagesCount );
    outSwapchain->imageViews.resize ( outSwapchain->imagesCount );
    outSwapchain->graphicssCommandBuffers.resize ( outSwapchain->imagesCount );

    VkResult result = vkGetSwapchainImagesKHR ( context->logicalDeviceInfo.handle, outSwapchain->handle, &outSwapchain->imagesCount, outSwapchain->images.data () );

    // create imageViews for swapchain images
    for ( uint32_t i = 0; i < outSwapchain->imagesCount; ++i )
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.format = outSwapchain->surfaceFormat.format;
        imageViewCreateInfo.image = outSwapchain->images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;

        VkResult imageViewResult = vkCreateImageView ( context->logicalDeviceInfo.handle, &imageViewCreateInfo, context->allocator, &outSwapchain->imageViews[i] );
    }

    // depth buffer
    {
        if ( !QueryDepthBufferFormat ( context, &context->physicalDeviceInfo.depthFormat ) )
        {
            context->physicalDeviceInfo.depthFormat = VK_FORMAT_UNDEFINED;
            Logger::Fatal ( "Couldn't find depth format" );
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
        Texture::Create ( context, descriptor, &outSwapchain->depthAttachement );
    }

    // we create a command buffer for each swap chain image
    {
        // free the command buffers if any are allocated and reallocate
        for ( uint32_t i = 0; i < outSwapchain->graphicssCommandBuffers.size (); ++i )
        {
            CommandBuffer* curr = &outSwapchain->graphicssCommandBuffers[i];

            if ( curr->handle != VK_NULL_HANDLE )
            {
                CommandBuffer::Free ( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, curr );
            }

            CommandBuffer::Allocate ( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, true, curr );
        }
    }


    // create sync objects
    {
        outSwapchain->imageAvailableSemaphore = std::vector<VkSemaphore> ( context->swapchainInfo.imagesCount );
        outSwapchain->finishedRenderingSemaphore = std::vector<VkSemaphore> ( context->swapchainInfo.imagesCount );
        outSwapchain->cmdBufferSumitFencePerFrameIndex = std::vector<Fence> ( context->swapchainInfo.imagesCount );

        for ( uint32_t i = 0; i < context->swapchainInfo.imagesCount; ++i )
        {
            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            // create semaphore for image available
            vkCreateSemaphore ( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &outSwapchain->imageAvailableSemaphore[i] );

            // create semaphore for queue complete
            vkCreateSemaphore ( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &outSwapchain->finishedRenderingSemaphore[i] );

            // the fences are initialized to true so that tha app doesn't block waiting for the first frame
            Fence::Create ( context, true, &outSwapchain->cmdBufferSumitFencePerFrameIndex[i] );
        }

        outSwapchain->fencePtrAssociatedPerImageIndex = std::vector<Fence*> ( context->swapchainInfo.imagesCount, nullptr );
    }

    

    return true;
}

bool SwapchainInfo::CreateFrameBuffers (VulkanContext* context)
{
    // create frame buffers
    {
        frameBuffers.resize ( imagesCount );

        for ( uint32_t i = 0; i < imagesCount; ++i )
        {
            std::vector<VkImageView> attachments = std::vector<VkImageView> ( 2 );
            attachments[0] = imageViews[i];
            attachments[1] = depthAttachement.view;

            FrameBuffer frameBuffer = {};
            FrameBuffer::Create ( context, &context->renderPass, context->frameBufferSize, &attachments, &frameBuffers[i] );
        }
    }

    return true;
}

bool SwapchainInfo::Recreate ( VulkanContext* context, SwapchainCreateDescription descrption, SwapchainInfo* outSwapchain )
{
    if ( context->recreateSwapchain )
    {
        return false;
    }

    if ( descrption.height == 0 || descrption.width == 0 )
    {
        return false;
    }

    context->recreateSwapchain = true;

    vkDeviceWaitIdle ( context->logicalDeviceInfo.handle );    

    {
        Clear ( context, outSwapchain );
        Destroy ( context, outSwapchain );
        Create ( context, descrption, outSwapchain );
        outSwapchain->CreateFrameBuffers ( context );
    }

    context->frameBufferSizeLastGeneration = context->frameBufferSizeCurrentGeneration;

    context->renderPass.area.x = 0;
    context->renderPass.area.y = 0;
    context->renderPass.area.width = context->frameBufferSize.x;
    context->renderPass.area.height = context->frameBufferSize.y;

    context->recreateSwapchain = false;

    return true;
}


bool SwapchainInfo::AcquireNextImageIndex ( VulkanContext* context, uint32_t timeOutMs, VkSemaphore semaphore, VkFence fence, uint32_t* outNextImageIndex )
{
    VkResult result = vkAcquireNextImageKHR (
        context->logicalDeviceInfo.handle,
        this->handle,
        timeOutMs,
        semaphore,
        fence, outNextImageIndex );


    if ( result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        SwapchainInfo::Destroy ( context, this );

        SwapchainCreateDescription desc = {};
        desc.width = context->frameBufferSize.x;
        desc.height = context->frameBufferSize.y;

        SwapchainInfo::Create ( context, desc , this );
        return false;
    }

    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        Logger::Fatal ( "Failed to acquire next image index" );
        return false;
    }

    return true;
}

bool SwapchainInfo::Present ( VulkanContext* context, VkSemaphore renderComplete, uint32_t* inPresentImageIndex )
{
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderComplete;
    presentInfo.pSwapchains = &this->handle;
    presentInfo.swapchainCount = 1;
    presentInfo.pImageIndices = inPresentImageIndex;
    presentInfo.pResults = nullptr;

    VkResult result = vkQueuePresentKHR ( context->physicalDeviceInfo.queuesInfo.presentQueue, &presentInfo );

    if ( result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        SwapchainInfo::Destroy ( context, &context->swapchainInfo );

        SwapchainCreateDescription desc = {};
        desc.width = context->frameBufferSize.x;
        desc.height = context->frameBufferSize.y;

        SwapchainInfo::Create ( context, desc, &context->swapchainInfo );
        return false;
    }

    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        Logger::Fatal ( "Failed to acquire swapchain" );
        return false;
    }

    // increments and loop frame count
    context->currentFrame = (context->currentFrame + 1) % maxFramesInFlight;

    return true;
}

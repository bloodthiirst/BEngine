#include <Maths/Maths.h>
#include "../../Global/Global.h"
#include "../../Logger/Logger.h"
#include "../Texture/Texture.h"
#include "../Fence/Fence.h"
#include "SwapchainInfo.h"
#include "VulkanContext.h"
#include "SwapchainSupportInfo.h"


bool QueryDepthBufferFormat( VulkanContext* context, VkFormat* outDepthFormat )
{
    VkFormat depthFormats[] = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    uint32_t flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    for ( uint32_t i = 0; i < context->swapchain_info.imagesCount; ++i )
    {
        VkFormatProperties formatProps = {};
        vkGetPhysicalDeviceFormatProperties( context->physicalDeviceInfo.handle, depthFormats[i], &formatProps );

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

void QuerySwapchainSupport( VkPhysicalDevice handle, VkSurfaceKHR surface, SwapchainSupportInfo* outSwapchainInfo )
{
    // capabilities
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( handle, surface, &capabilities );

    Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

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

bool AllocateResource( VulkanContext* context, SwapchainInfo* outSwapchain )
{
    uint32_t image_count = outSwapchain->imagesCount;

    Allocator alloc = Global::alloc_toolbox.heap_allocator;
    DArray<VkImage>::Create( image_count, &outSwapchain->images, alloc );
    outSwapchain->images.size = image_count;

    DArray<VkImageView>::Create( image_count, &outSwapchain->imageViews, alloc );
    outSwapchain->imageViews.size = image_count;

    DArray<CommandBuffer>::Create( image_count, &outSwapchain->graphics_cmd_buffers_per_image, alloc );
    outSwapchain->graphics_cmd_buffers_per_image.size = image_count;

    VkResult result = vkGetSwapchainImagesKHR( context->logicalDeviceInfo.handle, outSwapchain->handle, &outSwapchain->imagesCount, outSwapchain->images.data );

    // create imageViews for swapchain images
    for ( uint32_t i = 0; i < outSwapchain->imagesCount; ++i )
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
        if ( !QueryDepthBufferFormat( context, &context->physicalDeviceInfo.depthFormat ) )
        {
            context->physicalDeviceInfo.depthFormat = VK_FORMAT_UNDEFINED;
            Global::logger.Fatal( "Couldn't find depth format" );
            return false;
        }

        // create depth buffer image
        TextureDescriptor descriptor = {};
        descriptor.format = context->physicalDeviceInfo.depthFormat;
        descriptor.width = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.currentExtent.width;
        descriptor.height = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.currentExtent.height;
        descriptor.image_type = VK_IMAGE_TYPE_2D;
        descriptor.tiling = VK_IMAGE_TILING_OPTIMAL;
        descriptor.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        descriptor.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        descriptor.create_view = true;
        descriptor.view_aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;

        Texture depthTexture = {};
        Texture::Create( context, descriptor, &outSwapchain->depthAttachement );
    }

    // we create a command buffer for each swap chain image
    {
        // free the command buffers if any are allocated and reallocate
        for ( uint32_t i = 0; i < outSwapchain->graphics_cmd_buffers_per_image.size; ++i )
        {
            CommandBuffer* curr = &outSwapchain->graphics_cmd_buffers_per_image.data[i];

            if ( curr->handle != VK_NULL_HANDLE )
            {
                CommandBuffer::Free( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, curr );
            }

            CommandBuffer::Allocate( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, true, curr );
        }
    }


    // create sync objects
    {
        Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

        DArray<VkSemaphore>::Create( image_count, &outSwapchain->image_presentation_complete_semaphores, heap_alloc );
        outSwapchain->image_presentation_complete_semaphores.size = image_count;

        DArray<VkSemaphore>::Create( context->swapchain_info.imagesCount, &outSwapchain->finished_rendering_semaphores, heap_alloc );
        outSwapchain->finished_rendering_semaphores.size = image_count;

        DArray<Fence>::Create( context->swapchain_info.imagesCount, &outSwapchain->cmd_buffer_done_execution_per_frame, heap_alloc );
        outSwapchain->cmd_buffer_done_execution_per_frame.size = image_count;

        for ( uint32_t i = 0; i < image_count; ++i )
        {
            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            // create semaphore for image available
            vkCreateSemaphore( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &outSwapchain->image_presentation_complete_semaphores.data[i] );

            // create semaphore for queue complete
            vkCreateSemaphore( context->logicalDeviceInfo.handle, &createInfo, context->allocator, &outSwapchain->finished_rendering_semaphores.data[i] );

            // the fences are initialized to true so that tha app doesn't block waiting for the first frame
            Fence::Create( context, true, &outSwapchain->cmd_buffer_done_execution_per_frame.data[i] );
        }

        DArray<Fence*>::Create( context->swapchain_info.imagesCount, &outSwapchain->in_flight_fence_per_image, heap_alloc );
        outSwapchain->in_flight_fence_per_image.size = image_count;
    }

    return true;
}

void FreeResources( VulkanContext* context, SwapchainInfo* outSwapchain )
{
    for ( uint32_t i = 0; i < outSwapchain->graphics_cmd_buffers_per_image.size; ++i )
    {
        CommandBuffer* curr = &outSwapchain->graphics_cmd_buffers_per_image.data[i];

        CommandBuffer::Free( context, context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, curr );
    }

    DArray<CommandBuffer>::Clear( &outSwapchain->graphics_cmd_buffers_per_image );


    for ( uint32_t i = 0; i < outSwapchain->in_flight_fence_per_image.size; ++i )
    {
        outSwapchain->in_flight_fence_per_image.data[i] = nullptr;
    }

    DArray<Fence*>::Clear( &outSwapchain->in_flight_fence_per_image );

    for ( uint32_t i = 0; i < context->swapchain_info.imagesCount; ++i )
    {
        if ( outSwapchain->image_presentation_complete_semaphores.data[i] )
        {
            vkDestroySemaphore( context->logicalDeviceInfo.handle, outSwapchain->image_presentation_complete_semaphores.data[i], context->allocator );
            outSwapchain->image_presentation_complete_semaphores.data[i] = nullptr;
        }

        if ( outSwapchain->finished_rendering_semaphores.data[i] )
        {
            vkDestroySemaphore( context->logicalDeviceInfo.handle, outSwapchain->finished_rendering_semaphores.data[i], context->allocator );
            outSwapchain->finished_rendering_semaphores.data[i] = nullptr;
        }

        Fence::Destroy( context, &outSwapchain->cmd_buffer_done_execution_per_frame.data[i] );
    }

    // destroy imageViews
    {
        for ( uint32_t i = 0; i < outSwapchain->imageViews.size; ++i )
        {
            vkDestroyImageView( context->logicalDeviceInfo.handle, outSwapchain->imageViews.data[i], context->allocator );
        }

        DArray<VkImageView>::Clear( &context->swapchain_info.imageViews );
    }
    DArray<VkSemaphore>::Clear( &outSwapchain->image_presentation_complete_semaphores );
    DArray<VkSemaphore>::Clear( &outSwapchain->finished_rendering_semaphores );
    DArray<Fence>::Clear( &outSwapchain->cmd_buffer_done_execution_per_frame );
}

bool SwapchainInfo::Destroy( VulkanContext* context, SwapchainInfo* outSwapchain )
{
    Texture::Destroy( context, &outSwapchain->depthAttachement );

    for ( uint32_t i = 0; i < outSwapchain->imageViews.size; ++i )
    {
        vkDestroyImageView( context->logicalDeviceInfo.handle, outSwapchain->imageViews.data[i], context->allocator );
    }

    DArray<VkImageView>::Destroy( &context->swapchain_info.imageViews );

    vkDestroySwapchainKHR( context->logicalDeviceInfo.handle, outSwapchain->handle, context->allocator );

    return true;
}

bool CreateInternal( VulkanContext* context, SwapchainCreateDescription description , VkSwapchainKHR old_swapchain , SwapchainInfo* out_swapchain )
{
    // query the swapchain support
    QuerySwapchainSupport( context->physicalDeviceInfo.handle, context->surface, &context->physicalDeviceInfo.swapchainSupportInfo );

    // we select the image format that the swapchain's image will use
    // first we try to get format => R8G8B8A8 and colorSpace =>  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // if we don't find it , then we default to the first available format given by the physical device
    {
        bool found = false;

        for ( uint32_t i = 0; i < context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats.size; ++i )
        {
            VkSurfaceFormatKHR* curr = &context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats.data[i];

            if ( curr->format == VK_FORMAT_R8G8B8A8_UNORM && curr->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
            {
                out_swapchain->surfaceFormat = *curr;
                found = true;
                break;
            }
        }

        if ( !found )
        {
            out_swapchain->surfaceFormat = context->physicalDeviceInfo.swapchainSupportInfo.surfaceFormats.data[0];
        }
    }

    // we select the swapchain's present mode
    // vulkan specifices that all GPU must support FIFO so we start with it as the default mode
    // we query the physical devices modes and try to select MAILBOX if it's supported
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    {
        for ( int i = 0; i < context->physicalDeviceInfo.swapchainSupportInfo.presentModes.size; ++i )
        {
            VkPresentModeKHR mode = context->physicalDeviceInfo.swapchainSupportInfo.presentModes.data[i];

            if ( mode == VK_PRESENT_MODE_MAILBOX_KHR )
            {
                presentMode = mode;
                break;
            }
        }
    }

    VkExtent2D swapchainExtent = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.currentExtent;

    VkExtent2D gpuMin = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.minImageExtent;
    VkExtent2D gpuMax = context->physicalDeviceInfo.swapchainSupportInfo.capabilities.maxImageExtent;

    swapchainExtent.width = Maths::Clamp( swapchainExtent.width, gpuMin.width, gpuMax.width );
    swapchainExtent.height = Maths::Clamp( swapchainExtent.height, gpuMin.height, gpuMax.height );

    // minImage count is usually 2 , so +1 gives 3
    uint32_t imagesCount = description.imagesCount;

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
    swapchainCreateInfo.oldSwapchain = old_swapchain;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // this deals with the case where the device can be in portait or horizontal mode
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

    VkResult createSwapchainResult = vkCreateSwapchainKHR( context->logicalDeviceInfo.handle, &swapchainCreateInfo, context->allocator, &out_swapchain->handle );

    context->current_frame = 0;
    out_swapchain->imagesCount = 0;

    // here we take the image count returned by vulkan even though we already specified a wanted count
    // we do this since the number returned can be less than the count we requested (for whatever reason) 
    VkResult getImagesResult = vkGetSwapchainImagesKHR( context->logicalDeviceInfo.handle, out_swapchain->handle, &out_swapchain->imagesCount, nullptr );

    out_swapchain->maxFramesInFlight = out_swapchain->imagesCount - 1;

    return true;
}


bool SwapchainInfo::Create( VulkanContext* context, SwapchainCreateDescription description, VkSwapchainKHR old_swapchain, SwapchainInfo* out_swapchain )
{
    if ( !CreateInternal( context, description, old_swapchain, out_swapchain ) )
    {
        return false;
    }

    AllocateResource( context, out_swapchain );
}

bool SwapchainInfo::Recreate( VulkanContext* context, SwapchainCreateDescription descrption, SwapchainInfo* outSwapchain )
{
    VkSwapchainKHR old_swap = context->swapchain_info.handle;

    if ( context->recreateSwapchain )
    {
        return false;
    }

    if ( descrption.height == 0 || descrption.width == 0 )
    {
        return false;
    }

    context->recreateSwapchain = true;

    vkDeviceWaitIdle( context->logicalDeviceInfo.handle );

    // recreate
    {
        FreeResources( context, outSwapchain );
        CreateInternal( context, descrption, old_swap, outSwapchain );
        AllocateResource( context, outSwapchain );
    }

    context->frameBufferSizeLastGeneration = context->frameBufferSizeCurrentGeneration;

    context->renderPass.on_resize(&context->renderPass);

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


    if ( result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        SwapchainCreateDescription desc = {};
        desc.width = context->frameBufferSize.x;
        desc.height = context->frameBufferSize.y;

        SwapchainInfo::Recreate( context, desc, &context->swapchain_info );
        return false;
    }

    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
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

    if ( result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        VkSwapchainKHR old_swap = context->swapchain_info.handle;

        SwapchainInfo::Destroy( context, &context->swapchain_info );

        SwapchainCreateDescription desc = {};
        desc.width = context->frameBufferSize.x;
        desc.height = context->frameBufferSize.y;

        SwapchainInfo::Create( context, desc, old_swap, &context->swapchain_info );
        return false;
    }

    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        Global::logger.Fatal( "Failed to acquire swapchain" );
        return false;
    }

    return true;
}

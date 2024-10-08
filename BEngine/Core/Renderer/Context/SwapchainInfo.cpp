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

    for ( uint32_t i = 0; i < context->swapchain_info.images_count; ++i )
    {
        VkFormatProperties formatProps = {};
        vkGetPhysicalDeviceFormatProperties( context->physical_device_info.handle, depthFormats[i], &formatProps );

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
    uint32_t image_count = outSwapchain->images_count;

    Allocator alloc = Global::alloc_toolbox.heap_allocator;
    DArray<VkImage>::Create( image_count, &outSwapchain->images, alloc );
    outSwapchain->images.size = image_count;

    DArray<VkImageView>::Create( image_count, &outSwapchain->imageViews, alloc );
    outSwapchain->imageViews.size = image_count;

    DArray<CommandBuffer>::Create( image_count, &outSwapchain->graphics_cmd_buffers_per_image, alloc );
    outSwapchain->graphics_cmd_buffers_per_image.size = image_count;

    VkResult result = vkGetSwapchainImagesKHR( context->logical_device_info.handle, outSwapchain->handle, &outSwapchain->images_count, outSwapchain->images.data );

    // create imageViews for swapchain images
    for ( uint32_t i = 0; i < outSwapchain->images_count; ++i )
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

        VkResult imageViewResult = vkCreateImageView( context->logical_device_info.handle, &imageViewCreateInfo, context->allocator, &outSwapchain->imageViews.data[i] );
    }

    // depth buffer format
    {
        if ( !QueryDepthBufferFormat( context, &context->physical_device_info.depthFormat ) )
        {
            context->physical_device_info.depthFormat = VK_FORMAT_UNDEFINED;
            Global::logger.Fatal( "Couldn't find depth format" );
            return false;
        }

        // create depth buffer image
        TextureDescriptor descriptor = {};
        descriptor.format = context->physical_device_info.depthFormat;
        descriptor.width = context->physical_device_info.swapchainSupportInfo.capabilities.currentExtent.width;
        descriptor.height = context->physical_device_info.swapchainSupportInfo.capabilities.currentExtent.height;
        descriptor.image_type = VK_IMAGE_TYPE_2D;
        descriptor.mipmaps_level = 4;
        descriptor.tiling = VK_IMAGE_TILING_OPTIMAL;
        descriptor.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        descriptor.memory_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        descriptor.create_view = true;
        descriptor.view_aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;

        Texture::Create(descriptor, &outSwapchain->depthAttachement );
    }

    // we create a command buffer for each swap chain image
    {
        // free the command buffers if any are allocated and reallocate
        for ( uint32_t i = 0; i < outSwapchain->graphics_cmd_buffers_per_image.size; ++i )
        {
            CommandBuffer* curr = &outSwapchain->graphics_cmd_buffers_per_image.data[i];

            if ( curr->handle != VK_NULL_HANDLE )
            {
                CommandBuffer::Free( context->physical_device_info.command_pools_info.graphicsCommandPool, curr );
            }

            CommandBuffer::Allocate( context->physical_device_info.command_pools_info.graphicsCommandPool, true, curr );
        }
    }


    // create sync objects
    {
        Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

        DArray<VkSemaphore>::Create( image_count, &outSwapchain->image_presentation_complete_semaphores, heap_alloc );
        outSwapchain->image_presentation_complete_semaphores.size = image_count;

        DArray<VkSemaphore>::Create( context->swapchain_info.images_count, &outSwapchain->finished_rendering_semaphores, heap_alloc );
        outSwapchain->finished_rendering_semaphores.size = image_count;

        DArray<Fence>::Create( context->swapchain_info.images_count, &outSwapchain->cmd_buffer_done_execution_per_frame, heap_alloc );
        outSwapchain->cmd_buffer_done_execution_per_frame.size = image_count;

        for ( uint32_t i = 0; i < image_count; ++i )
        {
            VkSemaphoreCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            // create semaphore for image available
            vkCreateSemaphore( context->logical_device_info.handle, &createInfo, context->allocator, &outSwapchain->image_presentation_complete_semaphores.data[i] );

            // create semaphore for queue complete
            vkCreateSemaphore( context->logical_device_info.handle, &createInfo, context->allocator, &outSwapchain->finished_rendering_semaphores.data[i] );

            // the fences are initialized to true so that tha app doesn't block waiting for the first frame
            Fence::Create( context, true, &outSwapchain->cmd_buffer_done_execution_per_frame.data[i] );
        }

        DArray<Fence*>::Create( context->swapchain_info.images_count, &outSwapchain->in_flight_fence_per_image, heap_alloc );
        outSwapchain->in_flight_fence_per_image.size = image_count;
    }

    return true;
}

void FreeResources( VulkanContext* context, SwapchainInfo* out_swapchain )
{
    for ( uint32_t i = 0; i < out_swapchain->graphics_cmd_buffers_per_image.size; ++i )
    {
        CommandBuffer* curr = &out_swapchain->graphics_cmd_buffers_per_image.data[i];

        CommandBuffer::Free( context->physical_device_info.command_pools_info.graphicsCommandPool, curr );
    }

    DArray<CommandBuffer>::Clear( &out_swapchain->graphics_cmd_buffers_per_image );


    for ( uint32_t i = 0; i < out_swapchain->in_flight_fence_per_image.size; ++i )
    {
        out_swapchain->in_flight_fence_per_image.data[i] = nullptr;
    }

    DArray<Fence*>::Clear( &out_swapchain->in_flight_fence_per_image );

    for ( uint32_t i = 0; i < context->swapchain_info.images_count; ++i )
    {
        if ( out_swapchain->image_presentation_complete_semaphores.data[i] )
        {
            vkDestroySemaphore( context->logical_device_info.handle, out_swapchain->image_presentation_complete_semaphores.data[i], context->allocator );
            out_swapchain->image_presentation_complete_semaphores.data[i] = nullptr;
        }

        if ( out_swapchain->finished_rendering_semaphores.data[i] )
        {
            vkDestroySemaphore( context->logical_device_info.handle, out_swapchain->finished_rendering_semaphores.data[i], context->allocator );
            out_swapchain->finished_rendering_semaphores.data[i] = nullptr;
        }

        Fence::Destroy( context, &out_swapchain->cmd_buffer_done_execution_per_frame.data[i] );
    }

    // destroy imageViews
    {
        for ( uint32_t i = 0; i < out_swapchain->imageViews.size; ++i )
        {
            vkDestroyImageView( context->logical_device_info.handle, out_swapchain->imageViews.data[i], context->allocator );
        }

        DArray<VkImageView>::Clear( &context->swapchain_info.imageViews );
    }
    DArray<VkSemaphore>::Clear( &out_swapchain->image_presentation_complete_semaphores );
    DArray<VkSemaphore>::Clear( &out_swapchain->finished_rendering_semaphores );
    DArray<Fence>::Clear( &out_swapchain->cmd_buffer_done_execution_per_frame );
    Texture::Destroy(&out_swapchain->depthAttachement );

}

bool SwapchainInfo::Destroy( VulkanContext* ctx, SwapchainInfo* out_swapchain )
{
    FreeResources(ctx , out_swapchain);
    vkDestroySwapchainKHR(ctx->logical_device_info.handle , out_swapchain->handle , ctx->allocator);
    return true;
}

bool CreateInternal( VulkanContext* ctx, SwapchainCreateDescription description , VkSwapchainKHR old_swapchain , SwapchainInfo* out_swapchain )
{
    // query the swapchain support
    QuerySwapchainSupport( ctx->physical_device_info.handle, ctx->surface, &ctx->physical_device_info.swapchainSupportInfo );

    // we select the image format that the swapchain's image will use
    // first we try to get format => R8G8B8A8 and colorSpace =>  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    // if we don't find it , then we default to the first available format given by the physical device
    {
        bool found = false;

        for ( uint32_t i = 0; i < ctx->physical_device_info.swapchainSupportInfo.surfaceFormats.size; ++i )
        {
            VkSurfaceFormatKHR* curr = &ctx->physical_device_info.swapchainSupportInfo.surfaceFormats.data[i];

            if ( curr->format == VK_FORMAT_R8G8B8A8_UNORM && curr->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
            {
                out_swapchain->surfaceFormat = *curr;
                found = true;
                break;
            }
        }

        if ( !found )
        {
            out_swapchain->surfaceFormat = ctx->physical_device_info.swapchainSupportInfo.surfaceFormats.data[0];
        }
    }

    // we select the swapchain's present mode
    // vulkan specifices that all GPU must support FIFO so we start with it as the default mode
    // we query the physical devices modes and try to select MAILBOX if it's supported
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    {
        for ( int i = 0; i < ctx->physical_device_info.swapchainSupportInfo.presentModes.size; ++i )
        {
            VkPresentModeKHR mode = ctx->physical_device_info.swapchainSupportInfo.presentModes.data[i];

            if ( mode == VK_PRESENT_MODE_MAILBOX_KHR )
            {
                presentMode = mode;
                break;
            }
        }
    }

    VkExtent2D swapchainExtent = ctx->physical_device_info.swapchainSupportInfo.capabilities.currentExtent;

    VkExtent2D gpuMin = ctx->physical_device_info.swapchainSupportInfo.capabilities.minImageExtent;
    VkExtent2D gpuMax = ctx->physical_device_info.swapchainSupportInfo.capabilities.maxImageExtent;

    swapchainExtent.width = Maths::Clamp( swapchainExtent.width, gpuMin.width, gpuMax.width );
    swapchainExtent.height = Maths::Clamp( swapchainExtent.height, gpuMin.height, gpuMax.height );

    // minImage count is usually 2 , so +1 gives 3
    uint32_t imagesCount = description.imagesCount;

    imagesCount = Maths::Clamp
    (
        imagesCount,
        ctx->physical_device_info.swapchainSupportInfo.capabilities.minImageCount,
        ctx->physical_device_info.swapchainSupportInfo.capabilities.maxImageCount
    );

    // create swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.minImageCount = imagesCount;
    swapchainCreateInfo.imageExtent = swapchainExtent;
    swapchainCreateInfo.imageFormat = ctx->swapchain_info.surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = ctx->swapchain_info.surfaceFormat.colorSpace;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.surface = ctx->surface;
    swapchainCreateInfo.oldSwapchain = old_swapchain;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // this deals with the case where the device can be in portait or horizontal mode
    swapchainCreateInfo.preTransform = ctx->physical_device_info.swapchainSupportInfo.capabilities.currentTransform;

    uint32_t queueFamilyIndicies[] =
    {
        ctx->physical_device_info.queues_info.presentQueueFamilyIndex,
        ctx->physical_device_info.queues_info.graphicsQueueIndex,
    };

    if ( ctx->physical_device_info.queues_info.presentQueueFamilyIndex != ctx->physical_device_info.queues_info.graphicsQueueIndex )
    {
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndicies;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    }
    else
    {
        swapchainCreateInfo.queueFamilyIndexCount = 1;
        swapchainCreateInfo.pQueueFamilyIndices = &ctx->physical_device_info.queues_info.presentQueueFamilyIndex;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult createSwapchainResult = vkCreateSwapchainKHR( ctx->logical_device_info.handle, &swapchainCreateInfo, ctx->allocator, &out_swapchain->handle );

    ctx->current_frame = 0;
    out_swapchain->images_count = 0;

    // here we take the image count returned by vulkan even though we already specified a wanted count
    // we do this since the number returned can be less than the count we requested (for whatever reason) 
    VkResult getImagesResult = vkGetSwapchainImagesKHR( ctx->logical_device_info.handle, out_swapchain->handle, &out_swapchain->images_count, nullptr );

    if(old_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(ctx->logical_device_info.handle , old_swapchain , ctx->allocator);
    }

    return true;
}


bool SwapchainInfo::Create( VulkanContext* context, SwapchainCreateDescription description, VkSwapchainKHR old_swapchain, SwapchainInfo* out_swapchain )
{
    if ( !CreateInternal( context, description, old_swapchain, out_swapchain ) )
    {
        return false;
    }

    AllocateResource( context, out_swapchain );

    return true;
}

bool SwapchainInfo::Recreate( VulkanContext* context, SwapchainCreateDescription descrption, SwapchainInfo* outSwapchain )
{
    VkSwapchainKHR old_swap = context->swapchain_info.handle;

    if ( descrption.height == 0 || descrption.width == 0 )
    {
        return false;
    }

    vkDeviceWaitIdle( context->logical_device_info.handle );

    // recreate
    {
        FreeResources( context, outSwapchain );
        CreateInternal( context, descrption, old_swap, outSwapchain );
        AllocateResource( context, outSwapchain );
    }

    return true;
}


bool SwapchainInfo::AcquireNextImageIndex( VulkanContext* context, uint32_t timeout_ms, VkSemaphore semaphore, VkFence fence, uint32_t* out_next_image_index )
{
    VkResult result = vkAcquireNextImageKHR(
        context->logical_device_info.handle,
        this->handle,
        timeout_ms,
        semaphore,
        fence, out_next_image_index );


    if ( result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        SwapchainCreateDescription desc = {};
        desc.width = context->frame_buffer_size.x;
        desc.height = context->frame_buffer_size.y;

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

    VkResult result = vkQueuePresentKHR( context->physical_device_info.queues_info.presentQueue, &presentInfo );

    if ( result == VK_ERROR_OUT_OF_DATE_KHR )
    {
        SwapchainCreateDescription desc = {};
        desc.width = context->frame_buffer_size.x;
        desc.height = context->frame_buffer_size.y;

        SwapchainInfo::Recreate( context, desc, &context->swapchain_info );
        return false;
    }

    if ( result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR )
    {
        Global::logger.Fatal( "Failed to acquire swapchain" );
        return false;
    }

    return true;
}

#include <vulkan/vulkan.h>
#include "../../Global/Global.h"
#include "../../Logger/Logger.h"
#include "../Context/VulkanContext.h"
#include "Fence.h"

void Fence::Create ( VulkanContext* context, bool isSignaled, Fence* outFence )
{
    VkFenceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if ( isSignaled )
    {
        createInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT;
    }

    vkCreateFence ( context->logical_device_info.handle, &createInfo, context->allocator, &outFence->handle );
    outFence->isSignaled = isSignaled;
}

void Fence::Destroy ( VulkanContext* context, Fence* outFence )
{
    if ( outFence->handle != 0 )
    {
        vkDestroyFence ( context->logical_device_info.handle, outFence->handle, context->allocator );
        outFence->handle = 0;
    }

    outFence->isSignaled = false;
}

bool Fence::Wait ( VulkanContext* context, uint64_t timeoutMs )
{
    if ( isSignaled )
        return true;

    VkResult result = vkWaitForFences ( context->logical_device_info.handle, 1, &handle, true, timeoutMs );

    switch ( result )
    {
        case VK_SUCCESS:
        {
            isSignaled = true;
            return true;
        }
        case VK_TIMEOUT:
        {
            Global::logger.Warning ( "Fence - timeout" );
            break;
        }
        case VK_ERROR_DEVICE_LOST:
        {
            Global::logger.Error ( "Fence - Device lost" );
            break;
        }
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            Global::logger.Error ( "Fence - Out of host memory" );
            break;
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            Global::logger.Error ( "Fence - Out of device memory" );
            break;
        }
        default:
        {
            Global::logger.Error ( "Fence - Error occured" );
            break;
        }
    }

    return false;

}

void Fence::Reset ( VulkanContext* context )
{
    if ( isSignaled )
    {
        vkResetFences ( context->logical_device_info.handle, 1, &handle );
        isSignaled = false;
    }
    
}
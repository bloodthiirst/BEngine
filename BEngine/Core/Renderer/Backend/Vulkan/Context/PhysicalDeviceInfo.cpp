#include "PhysicalDeviceInfo.h"
#include "VulkanContext.h"

bool PhysicalDeviceInfo::FindMemoryIndex ( uint32_t typeFilter, uint32_t propertyFlags, uint32_t* outMemeoryIndex )
{
    for ( uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i )
    {
        VkMemoryType memory = physicalDeviceMemoryProperties.memoryTypes[i];

        if ( typeFilter & (1 << i) && (memory.propertyFlags & propertyFlags) == propertyFlags )
        {
            *outMemeoryIndex = i;
            return true;
        }
    }

    return false;
}
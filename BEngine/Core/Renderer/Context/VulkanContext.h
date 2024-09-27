#pragma once
#include <vulkan/vulkan.h>
#include <Maths/Vector2Int.h>
#include <Containers/FreeList.h>
#include <Maths/Rect.h>
#include <Maths/Color.h>
#include "LogicalDeviceInfo.h"
#include "PhysicalDeviceInfo.h"
#include "SwapchainInfo.h"
#include "../../Defines/Defines.h"
#include "../CommandBuffer/CommandBuffer.h"
#include "../Renderpass/Renderpass.h"
#include "../Fence/Fence.h"
#include "../DescriptorManager/DescriptorManager.h"
#include "../Texture/Texture.h"
#include "../Shader/Shader.h"
#include "../Buffer/Buffer.h"


struct BAPI VulkanContext
{
#if defined(_DEBUG)
	VkDebugUtilsMessengerEXT debug_messenger;
#endif
	VkAllocationCallbacks* allocator;
	VkInstance vulkan_instance;
	VkSurfaceKHR surface;

	LogicalDeviceInfo logical_device_info;
	PhysicalDeviceInfo physical_device_info;
	SwapchainInfo swapchain_info;
    DArray<Renderpass> renderpasses;
    
    DescriptorManager descriptor_manager;

    Buffer mesh_buffer;
	FreeList mesh_freelist;
	
    VkSampler default_sampler;

	/// <summary>
	/// <para>The index of the image that we're showing out the images provided by the swapchain</para>
	/// <para>We can use this index to get the image from swapchainInfo.images </para>
	/// </summary>
	uint32_t current_image_index;

	/// <summary>
	/// Always between 0 and swapchain.maxFramesInFlight 
	/// </summary>
	uint32_t current_frame;
	bool recreate_swapchain;

	Vector2Int frameBufferSize;
    uint32_t frameBufferSizeCurrentGeneration;
    uint32_t frameBufferSizeLastGeneration;
};


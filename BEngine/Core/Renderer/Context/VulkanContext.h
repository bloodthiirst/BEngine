#pragma once
#include <vulkan/vulkan.h>
#include <Maths/Vector2Int.h>
#include <Maths/Rect.h>
#include <Maths/Color.h>
#include "LogicalDeviceInfo.h"
#include "PhysicalDeviceInfo.h"
#include "SwapchainInfo.h"
#include "../CommandBuffer/CommandBuffer.h"
#include "../Renderpass/Renderpass.h"
#include "../Fence/Fence.h"
#include "../DescriptorManager/DescriptorManager.h"
#include "../Texture/Texture.h"
#include "../Shader/Shader.h"
#include "../Buffer/Buffer.h"


struct VulkanContext
{
public:
#if defined(_DEBUG)
	VkDebugUtilsMessengerEXT debugMessenger;
#endif
	VkAllocationCallbacks* allocator;
	VkInstance vulkanInstance;
	VkSurfaceKHR surface;

	LogicalDeviceInfo logicalDeviceInfo;
	PhysicalDeviceInfo physicalDeviceInfo;
	SwapchainInfo swapchain_info;
    DArray<Renderpass> renderPasses;
    
    DescriptorManager descriptor_manager;

    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t geometryVertexOffset;
    uint32_t geometryIndexOffset;
    uint32_t indexBufferSize;
    
    Shader default_shader;

    Texture default_texture;

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
	bool recreateSwapchain;

	Vector2Int frameBufferSize;
    uint32_t frameBufferSizeCurrentGeneration;
    uint32_t frameBufferSizeLastGeneration;
};


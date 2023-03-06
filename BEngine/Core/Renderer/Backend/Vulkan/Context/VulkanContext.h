#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "LogicalDeviceInfo.h"
#include "PhysicalDeviceInfo.h"
#include "SwapchainInfo.h"
#include "CommandBuffer.h"
#include "Renderpass.h"
#include "../../../../Maths/Vector2Int.h"
#include "../../../../Maths/Rect.h"
#include "../../../../Maths/Color.h"
#include "../../../Frontend/Texture/Texture.h"
#include "Fence.h"
#include "../../../Frontend/Shader/Shader.h"
#include "../../../Frontend/Buffer/Buffer.h"


class VulkanContext
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
	SwapchainInfo swapchainInfo;
    Renderpass renderPass;

    Buffer vertexBuffer;
    Buffer indexBuffer;
    uint32_t geometryVertexOffset;
    uint32_t geometryIndexOffset;
    uint32_t indexBufferSize;
    
    Shader defaultShader;

	/// <summary>
	/// <para>The index of the image that we're showing out the images provided by the swapchain</para>
	/// <para>We can use this index to get the image from swapchainInfo.images </para>
	/// </summary>
	uint32_t currentImageIndex;

	/// <summary>
	/// Always between 0 and swapchain.maxFramesInFlight 
	/// </summary>
	uint32_t currentFrame;
	bool recreateSwapchain;

	Vector2Int frameBufferSize;
    uint32_t frameBufferSizeCurrentGeneration;
    uint32_t frameBufferSizeLastGeneration;
};


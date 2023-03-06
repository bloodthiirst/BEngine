#pragma once
#include "../../../../Logger/Logger.h"
#include "../../BackendRenderer.h"
#include "../../../Context/RendererContext.h"
#include "../Context/VulkanContext.h"
#include <vulkan/vulkan.h>

class VulkanBackendRenderer : public BackendRenderer
{
	using BackendRenderer::BackendRenderer;
private:
	VulkanContext context;

public:
	VulkanBackendRenderer ( Application* app, Platform* platform );

	virtual void Resize ( uint32_t width, uint32_t height ) override;
	static VKAPI_ATTR VkBool32 DebugCallback ( VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* callbackData, void* userData );
	virtual bool Startup () override;
	virtual bool StartFrame ( RendererContext rendererContext ) override;
	virtual bool EndFrame ( RendererContext rendererContext ) override;
	virtual void Destroy () override;
	bool CreateSurface ( VulkanContext* context, VkSurfaceKHR* surface );
};


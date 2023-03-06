#pragma once
#include <vector>
#include <vulkan/vulkan.h>
#include "FrameBuffer.h"
#include "../../../Frontend/Texture/Texture.h"

struct SwapchainSupportInfo;
struct Fence;

struct SwapchainCreateDescription
{
public:
    uint32_t width;
    uint32_t height;
    uint32_t imagesCount;
};

struct SwapchainInfo
{
public:
    VkSurfaceFormatKHR surfaceFormat;
    VkSwapchainKHR handle;
    VkPresentModeKHR presentMode;
    uint32_t maxFramesInFlight;
    uint32_t imagesCount;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    std::vector<FrameBuffer> frameBuffers;
    std::vector<CommandBuffer> graphicssCommandBuffers;

    // sync objects
    std::vector<VkSemaphore> imageAvailableSemaphore;
    std::vector<VkSemaphore> finishedRenderingSemaphore;
    std::vector<Fence> cmdBufferSumitFencePerFrameIndex;
    std::vector<Fence*> fencePtrAssociatedPerImageIndex;

    Texture depthAttachement;
public:
    static void QuerySwapchainSupport ( VkPhysicalDevice handle, VkSurfaceKHR surface, SwapchainSupportInfo* outSwapchainInfo );
    static bool Destroy ( VulkanContext* context, SwapchainInfo* outSwapchain );
    static void Clear ( VulkanContext* context, SwapchainInfo* outSwapchain );
    static bool Create ( VulkanContext* context, SwapchainCreateDescription descrption, SwapchainInfo* outSwapchain );
    static bool Recreate ( VulkanContext* context, SwapchainCreateDescription descrption,SwapchainInfo* outSwapchain );
public:
    bool CreateFrameBuffers ( VulkanContext* context );
    bool AcquireNextImageIndex ( VulkanContext* context, uint32_t timeOutMs, VkSemaphore semaphore, VkFence fence, uint32_t* outNextImageIndex );
    bool Present ( VulkanContext* context, VkSemaphore renderComplete, uint32_t* outPresentImageIndex );
};



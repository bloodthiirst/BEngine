#pragma once
#include <Containers/DArray.h>
#include <vulkan/vulkan.h>
#include "../FrameBuffer/FrameBuffer.h"
#include "../Texture/Texture.h"

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
    VkSwapchainKHR handle;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;

    uint32_t images_count;
    DArray<VkImage> images;
    DArray<VkImageView> imageViews;

    /// <summary>
    /// <para>Array of command buffers associated with each swapchain image used for</para>
    /// <para>Use context.current_image_index as index to get the current command buffer</para>
    /// </summary>
    DArray<CommandBuffer> graphics_cmd_buffers_per_image;

    // sync objects
    DArray<VkSemaphore> image_presentation_complete_semaphores;
    DArray<VkSemaphore> finished_rendering_semaphores;

    /// <summary>
    /// <para>An array of fences used to signal to the CPU once the command buffer OR the presentation passed to the queue have completed execution</para>
    /// <para>Use context.current_frame as index to get the current Fence</para>
    /// </summary>
    DArray<Fence> cmd_buffer_done_execution_per_frame;

    /// <summary>
    /// <para>An array of pointers to fences used to go from image_index to the corresponding </para>
    /// <para>The fence in this array point to the same fences in cmd_buffer_done_execution_per_frame , they are just recycled after to be used for waiting on presentation </para>
    /// </summary>
    DArray<Fence*> in_flight_fence_per_image;

    Texture depthAttachement;
     
public:
    static bool Destroy ( VulkanContext* context, SwapchainInfo* outSwapchain );
    static bool Create ( VulkanContext* context, SwapchainCreateDescription descrption, VkSwapchainKHR old_swapchain, SwapchainInfo* outSwapchain );
    static bool Recreate ( VulkanContext* context, SwapchainCreateDescription descrption,SwapchainInfo* outSwapchain );
public:
    bool AcquireNextImageIndex ( VulkanContext* context, uint32_t timeOutMs, VkSemaphore semaphore, VkFence fence, uint32_t* outNextImageIndex );
    bool Present ( VulkanContext* context, VkSemaphore renderComplete, uint32_t* outPresentImageIndex );
};



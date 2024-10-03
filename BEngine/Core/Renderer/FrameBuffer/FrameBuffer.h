#pragma once
#include <Containers/DArray.h>
#include <Maths/Vector2Int.h>
#include <vulkan/vulkan.h>

struct Renderpass;
struct VulkanContext;

struct FrameBuffer
{
public:
    VkFramebuffer handle;
    DArray<VkImageView> attatchments;
    Renderpass* renderpass;

public:
    static void Create ( VulkanContext* context, Renderpass* in_renderpass, Vector2Int dimensions, DArray<VkImageView> attatchments, FrameBuffer* out_framebuffer );
    static void Destroy ( VulkanContext* context, FrameBuffer* outFramebuffer );
};


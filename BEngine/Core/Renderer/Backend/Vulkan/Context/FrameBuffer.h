#pragma once
#include <Containers/DArray.h>
#include <Maths/Vector2Int.h>
#include <vulkan/vulkan.h>
#include "Renderpass.h"

struct FrameBuffer
{
public:
    VkFramebuffer handle;
    DArray<VkImageView> attatchments;
    Renderpass* renderpass;

public:
    static void Create ( VulkanContext* context, Renderpass* renderpass, Vector2Int dimensions, DArray<VkImageView> attatchments, FrameBuffer* outFramebuffer );
    static void Destroy ( VulkanContext* context, FrameBuffer* outFramebuffer );
};


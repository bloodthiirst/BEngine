#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Renderpass.h"
#include "../../../../Maths/Vector2Int.h"

class FrameBuffer
{
public:
    VkFramebuffer handle;
    std::vector<VkImageView> attatchments;
    Renderpass* renderpass;

public:
    static void Create ( VulkanContext* context, Renderpass* renderpass, Vector2Int dimensions, std::vector<VkImageView>* attatchments, FrameBuffer* outFramebuffer );
    static void Destroy ( VulkanContext* context, FrameBuffer* outFramebuffer );
};


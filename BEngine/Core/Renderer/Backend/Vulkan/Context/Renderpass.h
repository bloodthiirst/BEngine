#pragma once
#include <vulkan/vulkan.h>
#include <Maths/Rect.h>
#include <Maths/Color.h>
#include "CommandBuffer.h"

enum class RenderpassState
{
    Ready,
    Recording,
    InRenderpass,
    RecordingEnded,
    Submitted,
    NoAllocated
};

struct VulkanContext;
struct FrameBuffer;

struct Renderpass
{
public:
    VkRenderPass handle;
    Rect area;
    Color clearColor;
    float depth;
    uint32_t stencil;
    RenderpassState state;

public:
    static bool Create ( VulkanContext* context, Rect rect, Color color, float depth, uint32_t stencil, Renderpass* renderpass );
    static void Destroy ( VulkanContext* context, Renderpass* renderpass );

public:
    void Begin ( VulkanContext* context , CommandBuffer* cmdBuffer, FrameBuffer* frameBuffer );
    void End ( VulkanContext* context, CommandBuffer* commandBuffer );
};

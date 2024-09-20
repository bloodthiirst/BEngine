#pragma once
#include <vulkan/vulkan.h>
#include <Maths/Rect.h>
#include <Maths/Color.h>
#include "../CommandBuffer/CommandBuffer.h"
#include "../Texture/Texture.h"
#include "../FrameBuffer/FrameBuffer.h"

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

struct RenderTarget
{
    Texture texture;
    FrameBuffer framebuffer;
};

struct Renderpass
{
    StringView id;
    VkRenderPass handle;
    void* internal_data;
    RenderpassState state;

    DArray<RenderTarget> render_targets;
    ActionParams<Renderpass*, CommandBuffer*> begin;
    ActionParams<Renderpass*, CommandBuffer* , RendererContext*> draw;
    ActionParams<Renderpass*, CommandBuffer*> end;
    ActionParams<Renderpass*> on_resize;
    ActionParams<Renderpass*> on_destroy;
};

#pragma once
#include <vulkan/vulkan.h>
#include <Maths/Rect.h>
#include <Maths/Color.h>
#include "../CommandBuffer/CommandBuffer.h"
#include "../Texture/Texture.h"
#include "../Subpasses/Subpass.h"
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

struct RenderTarget
{
    Texture texture;
    FrameBuffer framebuffer;
};

struct RendererContext;
struct CommandBuffer;

struct Renderpass
{
    StringView id;
    RenderGraph* graph;
    VkRenderPass handle;
    void* internal_data;
    RenderpassState state;
    DArray<Subpass> subpasses;

    DArray<RenderTarget> render_targets;
    ActionParams<Renderpass*, CommandBuffer*> begin;
    ActionParams<Renderpass*, CommandBuffer* , RendererContext*> draw;
    ActionParams<Renderpass*, CommandBuffer*> end;
    ActionParams<Renderpass*> on_resize;
    ActionParams<Renderpass*> on_destroy;
};

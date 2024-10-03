#pragma once
#include <vulkan/vulkan.h>
#include <String/StringView.h>
#include <Typedefs/Typedefs.h>
#include "../CommandBuffer/CommandBuffer.h"
#include "../Context/RendererContext.h"

struct RenderGraph;

struct Subpass
{
    StringView id;
    size_t renderpass_index;
    RenderGraph* graph;
    VkSubpassBeginInfo handle;
    void* internal_data;

    ActionParams<Subpass*, CommandBuffer*> begin;
    ActionParams<Subpass*, CommandBuffer* , RendererContext*> draw;
    ActionParams<Subpass*, CommandBuffer*> end;
    ActionParams<Subpass*> on_resize;
    ActionParams<Subpass*> on_destroy;
};

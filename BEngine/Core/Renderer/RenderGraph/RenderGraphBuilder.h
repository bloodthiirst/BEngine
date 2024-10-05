#pragma once
#include <Containers/HMap.h>
#include <Containers/DArray.h>
#include <Typedefs/Typedefs.h>
#include <assert.h>
#include "../Texture/Texture.h"
#include "../FrameBuffer/FrameBuffer.h"
#include "../Renderpass/Renderpass.h"
#include "../Subpasses/Subpass.h"

struct RenderGraphBuilder;
struct RenderpassNode;
struct RenderpassResult;

typedef Func<Renderpass , RenderGraphBuilder* , RenderGraph*,RenderpassNode> RenderpassBuilder;

struct RenderGraph
{
    HMap<StringView , Texture> all_textures;
    DArray<Renderpass> renderpasses; 
};

struct RenderGraphBuilder
{
    DArray<RenderpassNode> renderpasses;
    Allocator alloc;

    static RenderGraphBuilder Create(Allocator alloc);
    RenderpassNode* AddRenderpass(StringView id , void* params, RenderpassBuilder renderpass_builder);
    void Build(RenderGraph* out_graph);
};

struct SubpassNode
{
    RenderpassNode* parent;
    DArray<SubpassNode*> dependencies;
    Subpass subpass;
};

struct RenderTargetInfo
{
    size_t index;
    VkAttachmentDescription description;
};

struct RenderpassNode
{
    StringView id;
    RenderpassBuilder builder;
    void* params;
    HMap<StringView,RenderTargetInfo> render_targets;
    DArray<SubpassNode> subpasses;
    RenderGraphBuilder* parent;

    RenderpassNode* AddSubpass(Subpass pass)
    {
        SubpassNode node = {};
        node.subpass = pass;
        node.parent = this;
        DArray<SubpassNode*>::Create(3 , &node.dependencies , parent->alloc);

        DArray<SubpassNode>::Add(&subpasses , node);
        return this;
    }

    RenderpassNode* AddRenderTarget(StringView id , size_t index , VkAttachmentDescription desc)
    {
        RenderTargetInfo info = {};
        info.index = index;
        info.description = desc;

        bool added = HMap<StringView,RenderTargetInfo>::TryAdd( &render_targets, id , info , nullptr);
        assert(added);

        return this;
    }

    RenderpassNode* AddDependency(size_t dependency_index , size_t parent_index)
    {
        return this;
    }

    RenderGraphBuilder* Done()
    {
        return parent;
    }
};



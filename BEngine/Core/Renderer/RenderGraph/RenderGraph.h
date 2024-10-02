#pragma once
#include <Containers/HMap.h>
#include <Containers/DArray.h>
#include "../Texture/Texture.h"
#include "../FrameBuffer/FrameBuffer.h"
#include "../Renderpass/Renderpass.h"

struct RenderGraph;
struct RenderpassNode;

struct Subpass
{
};

struct SubpassNode
{
    RenderpassNode* parent;
    DArray<SubpassNode*> dependencies;
    Subpass subpass;
};

struct RenderpassInfo
{
};

struct RenderpassNode
{
    DArray<SubpassNode> subpasses;
    RenderGraph* parent;

    RenderpassNode* AddSubpass(SubpassNode node)
    {
        node.parent = this;
        DArray<SubpassNode>::Add(&subpasses , node);
        return this;
    }

    RenderpassNode* AddDependency(size_t dependency_index , size_t parent_index)
    {
        return this;
    }

    RenderGraph* Done()
    {
        return parent;
    }
};

struct RenderGraph
{
    HMap<StringView,Texture> global_textures;
    DArray<RenderpassNode> renderpasses;
    Allocator alloc;

    static RenderGraph Create(Allocator alloc)
    {
        RenderGraph graph = {};
        graph.alloc = alloc;
        DArray<RenderpassNode>::Create(10 , &graph.renderpasses , alloc );
        HMap<StringView,Texture>::Create(&graph.global_textures , alloc , 10 , 10 , StringUtils::Hash , StringUtils::Compare);

        return graph;
    }

    RenderpassNode* AddRenderpass()
    {
        RenderpassNode renderpass = {};
        DArray<SubpassNode>::Create(10 , &renderpass.subpasses , alloc );
        renderpass.parent = this;

        size_t index = renderpasses.size;
        DArray<RenderpassNode>::Add(&renderpasses , renderpass);

        RenderpassNode* renderpass_ptr = &renderpasses.data[index];
        return renderpass_ptr;
    }

};
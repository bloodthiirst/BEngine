#include "RenderGraphBuilder.h"

RenderGraphBuilder RenderGraphBuilder::Create(Allocator alloc)
{
    RenderGraphBuilder graph = {};
    graph.alloc = alloc;
    DArray<RenderpassNode>::Create(10, &graph.renderpasses, alloc);

    return graph;
}

RenderpassNode* RenderGraphBuilder::AddRenderpass(StringView id , void* params, RenderpassBuilder renderpass_builder)
{
    RenderpassNode renderpass_node = {};
    renderpass_node.id = id;
    renderpass_node.params = params;
    renderpass_node.builder = renderpass_builder;
    renderpass_node.parent = this;

    DArray<SubpassNode>::Create(10, &renderpass_node.subpasses, alloc);
    HMap<StringView,RenderTargetInfo>::Create(&renderpass_node.render_targets, alloc , 10, StringUtils::Hash , StringUtils::Compare);

    size_t index = renderpasses.size;
    DArray<RenderpassNode>::Add(&renderpasses, renderpass_node);

    RenderpassNode *renderpass_ptr = &renderpasses.data[index];
    return renderpass_ptr;
}

void RenderGraphBuilder::Build(RenderGraph* out_graph)
{
    *out_graph = {};
    DArray<Renderpass>::Create(10 , &out_graph->renderpasses , Global::alloc_toolbox.heap_allocator); 
    HMap<StringView, Texture>::Create(&out_graph->all_textures, alloc, 10, StringUtils::Hash, StringUtils::Compare);

    for (size_t renderpass_idx = 0; renderpass_idx < renderpasses.size; ++renderpass_idx)
    {
        RenderpassNode curr_rp = renderpasses.data[renderpass_idx];      
        
        Renderpass rp = curr_rp.builder(this , out_graph , curr_rp);
        rp.graph = out_graph;

        for (size_t subpass_idx = 0; subpass_idx < curr_rp.subpasses.size; subpass_idx++)
        {
            SubpassNode curr_sp = curr_rp.subpasses.data[subpass_idx];

            Subpass sp = curr_sp.subpass;
            sp.graph = out_graph;
            sp.renderpass_index = out_graph->renderpasses.size;

            DArray<Subpass>::Add(&rp.subpasses , sp);
        }

        DArray<Renderpass>::Add(&out_graph->renderpasses , rp);
    }
}
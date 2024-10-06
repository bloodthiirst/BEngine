#include <Maths/Vector3.h>
#include "Pipeline.h"
#include "../Context/VulkanContext.h"


bool Pipeline::Create( VulkanContext* context, Renderpass* renderpass, PipelineDependencies* in_dependencies, ShaderBuilder* builder, Pipeline* outPipeline )
{
    ArenaCheckpoint arena = Global::alloc_toolbox.GetArenaCheckpoint();
    Allocator temp_alloc = Global::alloc_toolbox.frame_allocator;

    // viewport
    VkPipelineViewportStateCreateInfo createPipelineViewport = {};
    createPipelineViewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createPipelineViewport.viewportCount = 1;
    createPipelineViewport.pViewports = &builder->viewport;
    createPipelineViewport.scissorCount = 1;
    createPipelineViewport.pScissors = &builder->scissor;

    // restarizer
    VkPipelineRasterizationStateCreateInfo createRasterizationInfo = {};
    createRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    createRasterizationInfo.polygonMode = builder->has_wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    createRasterizationInfo.lineWidth = 1.0f;
    createRasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    createRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    createRasterizationInfo.depthClampEnable = VK_FALSE;
    createRasterizationInfo.depthBiasEnable = VK_FALSE;
    createRasterizationInfo.depthBiasConstantFactor = 0.0f;
    createRasterizationInfo.depthBiasClamp = 0.0f;
    createRasterizationInfo.depthBiasSlopeFactor = 0.0f;

    // multisampling
    VkPipelineMultisampleStateCreateInfo multiSamplingCreateInfo = {};
    multiSamplingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSamplingCreateInfo.sampleShadingEnable = VK_FALSE;
    multiSamplingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multiSamplingCreateInfo.minSampleShading = 1.0f;
    multiSamplingCreateInfo.pSampleMask = nullptr;
    multiSamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multiSamplingCreateInfo.alphaToOneEnable = VK_FALSE;

    // depth and stencil
    VkPipelineDepthStencilStateCreateInfo depthAndStencilCreateInfo = {};
    depthAndStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthAndStencilCreateInfo.depthTestEnable = VK_FALSE;
    depthAndStencilCreateInfo.depthWriteEnable = VK_TRUE;
    depthAndStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthAndStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthAndStencilCreateInfo.stencilTestEnable = VK_FALSE;

    // blending
    // controls the way colors are blended into the image
    VkPipelineColorBlendAttachmentState colorBlendAttachementInfo = {};
    colorBlendAttachementInfo.blendEnable = VK_TRUE;
    colorBlendAttachementInfo.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachementInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachementInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachementInfo.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachementInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachementInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachementInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = {};
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachementInfo;

    // dynamic state
    const uint32_t dynamicsStateCount = 3;
    VkDynamicState dynamicState[dynamicsStateCount] =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = dynamicsStateCount;
    dynamicStateCreateInfo.pDynamicStates = dynamicState;

    // attributes
    // attributes map to "layout (location = i)" in the shader
    // example :
    // layout ( location = 0) in vec3 in_position;
    // layout ( location = 1) in vec2 in_texcoord;
    DArray<VkVertexInputAttributeDescription> vertexAttributes = {};
    DArray<VkVertexInputAttributeDescription>::Create( builder->vertex_attributes.size, &vertexAttributes, temp_alloc );

    uint32_t offset = 0;

    for ( size_t i = 0; i < builder->vertex_attributes.size; ++i )
    {
        VertexAttributeInfo* curr = &builder->vertex_attributes.data[i];

        VkVertexInputAttributeDescription description = {};
        description.binding = 0;
        description.format = curr->format;
        description.offset = offset;
        description.location = (uint32_t) curr->location;

        // this can be used for both offset and accumulating the total size
        offset += (uint32_t) curr->size;

        DArray<VkVertexInputAttributeDescription>::Add( &vertexAttributes, description );
    }

    // binding index
    VkVertexInputBindingDescription binding = {};
    binding.binding = 0;
    binding.stride = offset;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo = {};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexStateCreateInfo.pVertexBindingDescriptions = &binding;
    vertexStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t) vertexAttributes.size;
    vertexStateCreateInfo.pVertexAttributeDescriptions = vertexAttributes.data;

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineLayout pipelineLayout = {};

    // pipeline descriptor set layouts
    {
        // NOTE : Set X refers to the X'th index of the array pSetLayouts. 
        // ex : Set 0 is the descriptor set pSetLayouts[0].
        // so make sure the order here matches the order in the shader
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = (uint32_t) in_dependencies->descriptor_set_layouts.size;
        pipelineLayoutCreateInfo.pSetLayouts = in_dependencies->descriptor_set_layouts.data;

        vkCreatePipelineLayout( context->logical_device_info.handle, &pipelineLayoutCreateInfo, context->allocator, &pipelineLayout );
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // stage count
    {
        DArray<VkPipelineShaderStageCreateInfo> tmp = {};
        DArray<VkPipelineShaderStageCreateInfo>::Create( in_dependencies->shader_info.size, &tmp, temp_alloc );

        for ( size_t i = 0; i < in_dependencies->shader_info.size; ++i )
        {
            PipelineShaderInfo* curr = &in_dependencies->shader_info.data[i];

            VkPipelineShaderStageCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            info.pName = StringView::ToCString( curr->function_name, temp_alloc );
            info.module = curr->handle;
            info.stage = curr->flags;

            DArray<VkPipelineShaderStageCreateInfo>::Add( &tmp, info );
        }

        graphicsPipelineCreateInfo.stageCount = (uint32_t) tmp.size;
        graphicsPipelineCreateInfo.pStages = tmp.data;
    }

    graphicsPipelineCreateInfo.pVertexInputState = &vertexStateCreateInfo;
    graphicsPipelineCreateInfo.pTessellationState = nullptr;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &createPipelineViewport;
    graphicsPipelineCreateInfo.pRasterizationState = &createRasterizationInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multiSamplingCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthAndStencilCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;

    graphicsPipelineCreateInfo.layout = pipelineLayout;

    graphicsPipelineCreateInfo.renderPass = renderpass->handle;

    VkPipeline pipeline = {};
    VkResult result = vkCreateGraphicsPipelines( context->logical_device_info.handle, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, context->allocator, &pipeline );

    Global::alloc_toolbox.ResetArenaOffset( &arena );

    if ( result != VK_SUCCESS )
    {
        return false;
    }

    outPipeline->layout = pipelineLayout;
    outPipeline->handle = pipeline;

    return true;
}

bool Pipeline::Destroy( VulkanContext* context, Pipeline* inPipeline )
{
    if ( !inPipeline )
        return false;

    if ( inPipeline->handle )
    {
        vkDestroyPipeline( context->logical_device_info.handle, inPipeline->handle, context->allocator );
        inPipeline->handle = {};
    }
    if ( inPipeline->layout )
    {
        vkDestroyPipelineLayout( context->logical_device_info.handle, inPipeline->layout, context->allocator );
        inPipeline->layout = {};
    }

    return true;
}

bool Pipeline::Bind( CommandBuffer* in_cmd_buffer, VkPipelineBindPoint bindpoint, Pipeline* in_pipeline )
{
    vkCmdBindPipeline( in_cmd_buffer->handle, bindpoint, in_pipeline->handle );
    return true;
}

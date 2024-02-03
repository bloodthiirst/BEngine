#include <Maths/Vector3.h>
#include "Pipeline.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"


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
    createRasterizationInfo.cullMode = VK_CULL_MODE_NONE;
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
    colorBlendAttachementInfo.blendEnable = VK_FALSE;
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
    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo = {};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;


    // attributes
    {
        DArray<VkVertexInputBindingDescription> attr = {};
        DArray<VkVertexInputBindingDescription>::Create( builder->vertex_attributes.size, &attr, temp_alloc );

        DArray<VkVertexInputAttributeDescription> desc = {};
        DArray<VkVertexInputAttributeDescription>::Create( builder->vertex_attributes.size, &desc, temp_alloc );

        uint32_t offset = 0;

        for ( size_t i = 0; i < builder->vertex_attributes.size; ++i )
        {
            VertexAttributeInfo* curr = &builder->vertex_attributes.data[i];

            // binding index
            VkVertexInputBindingDescription binding = {};
            binding.binding = (uint32_t) curr->binding;
            binding.stride = (uint32_t) curr->size;
            binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            VkVertexInputAttributeDescription description = {};
            description.binding = (uint32_t) curr->binding;
            description.format = curr->format;
            description.offset = offset;
            description.location = 0;

            offset += (uint32_t) curr->size;

            DArray<VkVertexInputBindingDescription>::Add( &attr, binding );
            DArray<VkVertexInputAttributeDescription>::Add( &desc, description );
        }
        vertexStateCreateInfo.vertexBindingDescriptionCount = (uint32_t) attr.size;
        vertexStateCreateInfo.pVertexBindingDescriptions = attr.data;
        vertexStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t) desc.size;
        vertexStateCreateInfo.pVertexAttributeDescriptions = desc.data;
    }

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineLayout pipelineLayout = {};

    // pipeline descriptor set layouts
    {
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = (uint32_t) in_dependencies->descriptor_set_layouts.size;
        pipelineLayoutCreateInfo.pSetLayouts = in_dependencies->descriptor_set_layouts.data;

        vkCreatePipelineLayout( context->logicalDeviceInfo.handle, &pipelineLayoutCreateInfo, context->allocator, &pipelineLayout );
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // stage count
    {
        DArray<VkPipelineShaderStageCreateInfo> tmp = {};
        DArray<VkPipelineShaderStageCreateInfo>::Create( in_dependencies->shader_info.size ,  &tmp , temp_alloc);

        for ( size_t i = 0; i < in_dependencies->shader_info.size; ++i )
        {
            PipelineShaderInfo* curr = &in_dependencies->shader_info.data[i];

            VkPipelineShaderStageCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            info.pName = StringView::ToCString(curr->function_name , temp_alloc);
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
    VkResult result = vkCreateGraphicsPipelines( context->logicalDeviceInfo.handle, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, context->allocator, &pipeline );

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
        vkDestroyPipeline( context->logicalDeviceInfo.handle, inPipeline->handle, context->allocator );
        inPipeline->handle = {};
    }
    if ( inPipeline->layout )
    {
        vkDestroyPipelineLayout( context->logicalDeviceInfo.handle, inPipeline->layout, context->allocator );
        inPipeline->layout = {};
    }

    return true;
}

bool Pipeline::Bind( CommandBuffer* inCmdBuffer, VkPipelineBindPoint bindPoint, Pipeline* inPipeline )
{
    vkCmdBindPipeline( inCmdBuffer->handle, bindPoint, inPipeline->handle );
    return true;
}

#include "Pipeline.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"
#include "../../../Maths/Vector3.h"

bool Pipeline::Create ( VulkanContext* context, Renderpass* renderpass, PipelineDescriptor descriptor, Pipeline* outPipeline )
{
    // viewport
    VkPipelineViewportStateCreateInfo createPipelineViewport = {};
    createPipelineViewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    createPipelineViewport.viewportCount = 1;
    createPipelineViewport.pViewports = &descriptor.viewport;
    createPipelineViewport.scissorCount = 1;
    createPipelineViewport.pScissors = &descriptor.scissor;

    // restarizer
    VkPipelineRasterizationStateCreateInfo createRasterizationInfo = {};
    createRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    createRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    createRasterizationInfo.polygonMode = descriptor.isWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
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

    VkVertexInputBindingDescription bindingDescription = {};

    // binding index
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof ( Vector3 );
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // attributes
    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo = {};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexStateCreateInfo.vertexAttributeDescriptionCount = descriptor.attributes.size ();
    vertexStateCreateInfo.pVertexAttributeDescriptions = descriptor.attributes.data ();

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = {};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // pipeline descriptor set layouts
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = descriptor.descriptorSetLayouts.size ();
    pipelineLayoutCreateInfo.pSetLayouts = descriptor.descriptorSetLayouts.data ();

    VkPipelineLayout pipelineLayout = {};
    vkCreatePipelineLayout ( context->logicalDeviceInfo.handle, &pipelineLayoutCreateInfo, context->allocator, &pipelineLayout );

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = descriptor.stages.size ();
    graphicsPipelineCreateInfo.pStages = descriptor.stages.data ();
    graphicsPipelineCreateInfo.pVertexInputState = &vertexStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;

    graphicsPipelineCreateInfo.pViewportState = &createPipelineViewport;
    graphicsPipelineCreateInfo.pRasterizationState = &createRasterizationInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multiSamplingCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthAndStencilCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.pTessellationState = nullptr;

    graphicsPipelineCreateInfo.layout = pipelineLayout;

    graphicsPipelineCreateInfo.renderPass = renderpass->handle;

    VkPipeline pipeline = {};
    VkResult result = vkCreateGraphicsPipelines ( context->logicalDeviceInfo.handle, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, context->allocator, &pipeline );

    if ( result != VK_SUCCESS )
    {
        return false;
    }

    outPipeline->layout = pipelineLayout;
    outPipeline->handle = pipeline;

    return true;
}

bool Pipeline::Destroy ( VulkanContext* context, Pipeline* inPipeline )
{
    if ( !inPipeline )
        return false;

    if ( inPipeline->handle )
    {
        vkDestroyPipeline ( context->logicalDeviceInfo.handle, inPipeline->handle , context->allocator );
        inPipeline->handle = {};
    }
    if ( inPipeline->layout )
    {
        vkDestroyPipelineLayout ( context->logicalDeviceInfo.handle, inPipeline->layout, context->allocator );
        inPipeline->layout = {};
    }

    return true;
}

bool Pipeline::Bind ( CommandBuffer* inCmdBuffer, VkPipelineBindPoint bindPoint, Pipeline* inPipeline )
{
    vkCmdBindPipeline ( inCmdBuffer->handle, bindPoint, inPipeline->handle );
    return true;
}

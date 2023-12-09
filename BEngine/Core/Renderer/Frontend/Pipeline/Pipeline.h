#pragma once
#include <vulkan/vulkan.h>
#include <Containers/DArray.h>

struct VulkanContext;
struct Renderpass;
struct CommandBuffer;

struct PipelineDescriptor
{
    DArray<VkVertexInputAttributeDescription> attributes;
    DArray<VkDescriptorSetLayout> descriptorSetLayouts;
    DArray<VkPipelineShaderStageCreateInfo> stages;
    VkViewport viewport;
    VkRect2D scissor;
    bool isWireframe;
};

class Pipeline
{
public:
    VkPipeline handle;
    VkPipelineLayout layout;

public:
    static bool Create ( VulkanContext* context, Renderpass* renderpass, PipelineDescriptor descriptor, Pipeline* outPipeline );
    static bool Destroy ( VulkanContext* context,Pipeline* inPipeline );
    static bool Bind ( CommandBuffer* inCmdBuffer , VkPipelineBindPoint bindPoint ,Pipeline* inPipeline );
};

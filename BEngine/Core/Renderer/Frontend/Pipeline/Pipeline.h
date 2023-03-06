#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;
class Renderpass;
class CommandBuffer;

struct PipelineDescriptor
{
    std::vector<VkVertexInputAttributeDescription> attributes;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkPipelineShaderStageCreateInfo> stages;
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

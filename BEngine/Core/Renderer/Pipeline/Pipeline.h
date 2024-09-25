#pragma once
#include <vulkan/vulkan.h>
#include <Containers/DArray.h>
#include <String/StringView.h>

struct VulkanContext;
struct Renderpass;
struct CommandBuffer;
struct ShaderBuilder;

struct PipelineShaderInfo
{
    VkShaderModule handle;
    VkShaderStageFlagBits flags;
    StringView function_name;
};

struct PipelineDependencies
{
    DArray<VkDescriptorSetLayout> descriptor_set_layouts;
    DArray<PipelineShaderInfo> shader_info;
};

struct Pipeline
{
public:
    VkPipeline handle;
    VkPipelineLayout layout;

public:
    static bool Create ( VulkanContext* context, Renderpass* renderpass, PipelineDependencies* in_dependencies , ShaderBuilder* builder, Pipeline* outPipeline );
    static bool Destroy ( VulkanContext* context,Pipeline* inPipeline );
    static bool Bind ( CommandBuffer* inCmdBuffer , VkPipelineBindPoint bindPoint ,Pipeline* inPipeline );
};

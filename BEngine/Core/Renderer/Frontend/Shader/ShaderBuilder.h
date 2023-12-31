#pragma once
#include <vulkan/vulkan.h>
#include <Containers/DArray.h>
#include "../../../Global/Global.h"
#include <String/StringView.h>

struct Shader;
struct VulkanContext;

enum ShaderStageType
{
    Vertex,
    Fragement,
    EnumLength
};


struct ShaderStage
{
    VkShaderModule handle;

    VkShaderModuleCreateInfo shader_create_info;

    VkPipelineShaderStageCreateInfo pipeline_stage_create_info;
};

struct ShaderBuilder
{
    struct DescriptorPair
    {
        size_t binding;
        size_t layout;
        VkDescriptorType type;
    };

private:

    bool stages_present[ShaderStageType::EnumLength];
    ShaderStage stage_info[ShaderStageType::EnumLength];
    DArray<DescriptorPair> descriptors;

public:
    static ShaderBuilder Get()
    {
        // todo : change the allocator or figure something out
        ShaderBuilder res = {};
        DArray<DescriptorPair>::Create( 0, &res.descriptors, Global::alloc_toolbox.heap_allocator );
        
        return res;
    }

    ShaderBuilder SetStage( ShaderStageType type, StringView code );
    ShaderBuilder AddBufferDescriptor( size_t layout, size_t binding, VkDescriptorType bindingStage );
    ShaderBuilder AddImageDescriptor( size_t layout, size_t binding, VkDescriptorType bindingStage );
    bool Build( VulkanContext* context, Shader* out_shader );
};

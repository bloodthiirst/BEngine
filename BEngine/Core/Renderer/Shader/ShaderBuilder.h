#pragma once
#include <vulkan/vulkan.h>
#include <Containers/DArray.h>
#include <String/StringView.h>
#include "../../Global/Global.h"

struct Shader;
struct VulkanContext;

enum ShaderStageType
{
    Vertex,
    Fragement,
    EnumLength
};

struct DescriptorBindingInfo
{
    StringView name;
    size_t binding_index;
    VkDescriptorType type;
    VkShaderStageFlagBits stage_usage;
};

struct DescriptorLayoutInfo
{
    size_t layout_index;
    DArray<DescriptorBindingInfo> bindings;
};

struct VertexAttributeInfo
{
    StringView name;
    size_t location;
    size_t size;
    VkFormat format;
};

struct ShaderStage
{
    StringView code;
    VkShaderStageFlagBits stage_flagbits;
};

struct ShaderBuilder
{
    bool has_wireframe;
    VkViewport viewport;
    VkRect2D scissor;
    DArray<ShaderStage> shader_stages;
    DArray<DescriptorLayoutInfo> descriptor_layouts;
    DArray<VertexAttributeInfo> vertex_attributes;
    
public:
    static ShaderBuilder Create()
    {
        ShaderBuilder res = {};
        DArray<DescriptorLayoutInfo>::Create( 0, &res.descriptor_layouts, Global::alloc_toolbox.heap_allocator );
        DArray<VertexAttributeInfo>::Create( 0, &res.vertex_attributes, Global::alloc_toolbox.heap_allocator );
        DArray<ShaderStage>::Create( 0, &res.shader_stages, Global::alloc_toolbox.heap_allocator );

        return res;
    }

    ShaderBuilder SetWireframe( bool has_wireframe )
    {
        this->has_wireframe = has_wireframe;
        return *this;
    }
    
    ShaderBuilder SetViewport( VkViewport viewport )
    {
        this->viewport = viewport;
        return *this;
    }

    ShaderBuilder SetScissor( VkRect2D scissor )
    {
        this->scissor = scissor;
        return *this;
    }

    ShaderBuilder SetStage( VkShaderStageFlagBits type, StringView code );
    ShaderBuilder AddDescriptor( StringView name, size_t layout, size_t binding, VkDescriptorType type, VkShaderStageFlagBits stage_usage );
    ShaderBuilder AddVertexAttribute( StringView name, size_t binding , size_t size, VkFormat format);
    bool Build( VulkanContext* context, Shader* out_shader );
};

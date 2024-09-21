#pragma once
#include <vulkan/vulkan.h>
#include <spirv_reflect.h>
#include <Containers/DArray.h>
#include <String/StringBuffer.h>
#include "../../Global/Global.h"

struct Shader;
struct VulkanContext;
struct Renderpass;

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
    StringBuffer code;
    VkShaderStageFlagBits stage_flagbits;
};

struct ShaderBuilder
{
    StringView name;
    bool has_wireframe;
    VkViewport viewport;
    VkRect2D scissor;
    DArray<ShaderStage> shader_stages;
    DArray<DescriptorLayoutInfo> descriptor_layouts;
    DArray<VertexAttributeInfo> vertex_attributes;

    static ShaderBuilder Create()
    {
        ShaderBuilder res = {};
        DArray<DescriptorLayoutInfo>::Create( 0, &res.descriptor_layouts, Global::alloc_toolbox.heap_allocator );
        DArray<VertexAttributeInfo>::Create( 0, &res.vertex_attributes, Global::alloc_toolbox.heap_allocator );
        DArray<ShaderStage>::Create( 0, &res.shader_stages, Global::alloc_toolbox.heap_allocator );

        return res;
    }

    static void Destroy(ShaderBuilder* builder)
    {
        for(size_t i=0; i < builder->shader_stages.size; ++i)
        {
            ShaderStage curr = builder->shader_stages.data[i];
            StringBuffer::Destroy(&curr.code);
        }

        for(size_t i=0; i < builder->descriptor_layouts.size; ++i)
        {
            DescriptorLayoutInfo curr = builder->descriptor_layouts.data[i];
            DArray<DescriptorBindingInfo>::Destroy(&curr.bindings);
        }

        DArray<ShaderStage>::Destroy(&builder->shader_stages);
        DArray<DescriptorLayoutInfo>::Destroy(&builder->descriptor_layouts);

        *builder = {};
    }

    ShaderBuilder SetName( StringView name )
    {
        this->name = name;
        return *this;
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

    ShaderBuilder SetStage( VkShaderStageFlagBits type, StringBuffer code );
    ShaderBuilder AddDescriptor( StringView name, size_t layout, size_t binding, VkDescriptorType type, VkShaderStageFlagBits stage_usage );
    ShaderBuilder AddVertexAttribute( StringView name, size_t binding , size_t size, VkFormat format);
    bool Build( VulkanContext* context, Renderpass* in_renderpass, Shader* out_shader );
};

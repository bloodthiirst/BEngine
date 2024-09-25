#pragma once
#include <vulkan/vulkan.h>
#include <spirv_reflect.h>
#include <Allocators/Allocator.h>
#include <Containers/DArray.h>
#include <String/StringBuffer.h>
#include "../../Global/Global.h"
#include "../../Defines/Defines.h"

struct VulkanContext;
struct Renderpass;
struct Shader;

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

struct BAPI ShaderBuilder
{
    StringView name;
    bool has_wireframe;
    VkViewport viewport;
    VkRect2D scissor;
    DArray<ShaderStage> shader_stages;
    DArray<DescriptorLayoutInfo> descriptor_layouts;
    DArray<VertexAttributeInfo> vertex_attributes;

    static ShaderBuilder Create();
    ShaderBuilder SetStage( VkShaderStageFlagBits type, StringBuffer code );
    ShaderBuilder AddDescriptor( StringView name, size_t layout, size_t binding, VkDescriptorType type, VkShaderStageFlagBits stage_usage );
    ShaderBuilder AddVertexAttribute( StringView name, size_t binding , size_t size, VkFormat format);
    bool Build( VulkanContext* context, Renderpass* in_renderpass, Shader* out_shader );

    static void Destroy(ShaderBuilder* builder);

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
};

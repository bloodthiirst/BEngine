#pragma once
#include <vulkan/vulkan.h>
#include <Maths/Matrix4x4.h>
#include "../../../Platform/Base/Memory.h"
#include "../../Frontend/Buffer/Buffer.h"
#include "../Pipeline/Pipeline.h"

struct Filesystem;
struct ShaderBuilder;
struct DescriptorPoolInfo;


struct GlobalUniformObject
{
    union
    {
        struct
        {
            char totalSize[256];
        };

        struct
        {
            Matrix4x4 projection;
            Matrix4x4 view;
        };
    };
};

struct Shader
{
    ShaderBuilder builder;

    VkShaderModule shader_modules[ShaderStageType::EnumLength];

    Pipeline pipeline;

    Buffer globalUBOBuffer;

    GlobalUniformObject global_UBO;

    /// <summary>
    /// Refers to the sets in the shader code
    /// We allocate one set per swapchain frame , just like the commands
    /// </summary>
    VkDescriptorSet globalDescriptorSets[3];

    VkDescriptorSet material_descriptor_sets[3];

    /// <summary>
    /// Pool used to create the descriptor sets
    /// </summary>
    DescriptorPoolInfo* globalDescriptorPool;

    /// <summary>
    /// Describes the layout of the data described by the descriptor set
    /// </summary>
    DArray<VkDescriptorSetLayout> descriptor_set_layouts;

public:
    static bool Destroy( VulkanContext* context, Shader* inShader );
    static bool UpdateGlobalBuffer( VulkanContext* context, GlobalUniformObject globalBuffer, Shader* inShader );
    static bool Bind( VulkanContext* context, Shader* inShader );
};




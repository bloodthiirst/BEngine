#pragma once
#include <vulkan/vulkan.h>
#include <Maths/Matrix4x4.h>
#include "../../Defines/Defines.h"
#include "../../Platform/Base/Memory.h"
#include "../Buffer/Buffer.h"
#include "../Pipeline/Pipeline.h"
#include "ShaderBuilder.h"

struct Filesystem;
struct DescriptorPoolInfo;
struct Texture;

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
            float time;
        };
    };
};

struct BAPI Shader
{
    ShaderBuilder builder;

    VkShaderModule shader_modules[ShaderStageType::EnumLength];

    Pipeline pipeline;

    /// <summary>
    /// Refers to the sets in the shader code
    /// We allocate one set per swapchain frame , just like the commands
    /// </summary>
    DArray<VkDescriptorSet> descriptor_sets[3];

    /// <summary>
    /// Pool used to create the descriptor sets
    /// </summary>
    DArray<DescriptorPoolInfo *> descriptor_pools;

    /// <summary>
    /// Describes the layout of the data described by the descriptor set
    /// </summary>
    DArray<VkDescriptorSetLayout> descriptor_set_layouts;

public:
    static bool Bind(VulkanContext *context, Shader *in_shader);
    static void SetBuffer(VulkanContext *context, Shader *in_shader, uint32_t descriptor_set_index, Buffer *in_buffer);
    static void SetTexture(VulkanContext *context, Shader *in_shader, uint32_t descriptor_set_index, Texture *in_texture);
    static bool Destroy(VulkanContext *context, Shader *in_shader);
};

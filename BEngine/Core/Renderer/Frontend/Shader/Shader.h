#pragma once
#include <vulkan/vulkan.h>
#include "../Pipeline/Pipeline.h"
#include <Maths/Matrix4x4.h>
#include "../../../Platform/Base/Memory.h"
#include "../../Frontend/Buffer/Buffer.h"
#define SHADER_STAGES 2
#define BUILTIN_SHADER_NAME "Builtin.ObjectShader"

struct VulkanContext;
struct Filesystem;

struct GlobalUniformObject
{
public:
    
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
struct ShaderStage
{
public:
    VkShaderModule handle;
    VkShaderModuleCreateInfo shaderCreateInfo;
    VkPipelineShaderStageCreateInfo stageCreateInfo;
};

class Shader
{
public:
    ShaderStage shaderStages[SHADER_STAGES];
    Pipeline pipeline;

    Buffer globalUBOBuffer;
    GlobalUniformObject global_UBO;

    /// <summary>
    /// Pool used to create the descriptor sets
    /// </summary>
    VkDescriptorPool globalDescriptorPool;

    /// <summary>
    /// Describes the layout of the data described by the descriptor set
    /// </summary>
    VkDescriptorSetLayout globalDescriptorSetLayout;

    /// <summary>
    /// Refers to the sets in the shader code
    /// We allocate one set per swapchain frame , just like the commands
    /// </summary>
    VkDescriptorSet globalDescriptorSets[3];



public:
    static bool Create ( VulkanContext* context, Filesystem* filesystem, Shader* outShader );
    static bool Destroy ( VulkanContext* context, Shader* inShader );
    static bool UpdateGlobalBuffer ( VulkanContext* context, Memory* memroy, GlobalUniformObject globalBuffer , Shader* inShader );
    static bool Bind ( VulkanContext* context, Shader* inShader );
};
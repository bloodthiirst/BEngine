#pragma once
#include <vulkan/vulkan.h>
#include "../Pipeline/Pipeline.h"
#define SHADER_STAGES 2
#define BUILTIN_SHADER_NAME "Builtin.ObjectShader"

class VulkanContext;
class Filesystem;

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

public:
    static bool Create ( VulkanContext* context, Filesystem* filesystem, Shader* outShader );
    static bool Destroy ( VulkanContext* context, Shader* inShader );
    static bool Bind ( VulkanContext* context, Shader* inShader );
};
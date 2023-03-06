#include "Shader.h"
#include "../../BEngine/BEngine/Core/Platform/Base/Filesystem/Filesystem.h"
#include "../../BEngine/BEngine/Core/Logger/Logger.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"
#include <string>
#include <format>
#include "../../../Maths/Vector3.h"

bool CreateShaderModule ( VulkanContext* context, Filesystem* filesystem, const std::string& shaderName, const std::string& shaderType, VkShaderStageFlagBits flag, uint32_t stageIndex, ShaderStage* shaderStagesArray )
{
    std::string filePath = shaderName + "." + shaderType + ".spv";

    ShaderStage* shaderStage = &shaderStagesArray[stageIndex];
    memset ( shaderStage, 0, sizeof ( ShaderStage ) );

    shaderStage->shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // load shader file
    FileHandle fHandle = {};
    if ( !filesystem->Open ( filePath, FileMode::Read, true, &fHandle ) )
    {
        Logger::Error ( std::format ( "Couldn't load shader file in {}", filePath ).c_str () );
        return false;
    }


    void* dataPtr = {};
    uint32_t dataSize = 0;
    if ( !filesystem->ReadAll ( fHandle, &dataPtr, &dataSize ) )
    {
        Logger::Error ( "Couldn't read file contents" );
        return false;
    }

    shaderStage->shaderCreateInfo.codeSize = dataSize;
    shaderStage->shaderCreateInfo.pCode = (uint32_t*) dataPtr;

    filesystem->Close ( fHandle );

    VkResult res = vkCreateShaderModule ( context->logicalDeviceInfo.handle, &shaderStage->shaderCreateInfo, context->allocator, &shaderStage->handle );

    if ( res != VK_SUCCESS )
    {
        Logger::Error ( "Shader module creation error" );
        return false;
    }

    shaderStage->stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage->stageCreateInfo.stage = flag;
    shaderStage->stageCreateInfo.module = shaderStage->handle;
    shaderStage->stageCreateInfo.pName = "main";

    if ( dataPtr )
    {
        free ( dataPtr );
    }

    return true;

}


bool Shader::Create ( VulkanContext* context, Filesystem* filesystem, Shader* outShader )
{
    std::string stageTypesStrings[SHADER_STAGES] = { "vert" , "frag" };

    VkShaderStageFlagBits stageTypes[SHADER_STAGES] = { VK_SHADER_STAGE_VERTEX_BIT , VK_SHADER_STAGE_FRAGMENT_BIT };

    for ( uint32_t i = 0; i < SHADER_STAGES; ++i )
    {
        std::string shaderType = stageTypesStrings[i];
        VkShaderStageFlagBits shaderStageBit = stageTypes[i];
        if ( !CreateShaderModule ( context, filesystem, BUILTIN_SHADER_NAME, shaderType, shaderStageBit, i, outShader->shaderStages ) )
        {
            Logger::Error ( "Create shader mode error" );
            return false;
        }
    }

    // todo : descriptors

    // create pipeline
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = context->frameBufferSize.y;
    viewport.width = context->frameBufferSize.x;
    viewport.height = -context->frameBufferSize.y;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context->frameBufferSize.x;
    scissor.extent.height = context->frameBufferSize.y;

    // attributes
    uint32_t offset = 0;
    const uint32_t attributeCount = 1;
    VkVertexInputAttributeDescription attrDescription[attributeCount];

    // position
    VkFormat formats[attributeCount] = {
        VK_FORMAT_R32G32B32_SFLOAT
    };

    uint32_t sizes[attributeCount] = {
        sizeof ( Vector3 )
    };

    for ( uint32_t i = 0; i < attributeCount; ++i )
    {
        attrDescription[i].binding = 0;
        attrDescription[i].location = i;
        attrDescription[i].format = formats[i];
        attrDescription[i].offset = offset;

        offset += sizes[i]; 
    }

    // stages

    VkPipelineShaderStageCreateInfo stagesCreateInfo[SHADER_STAGES];

    for ( uint32_t i = 0; i < SHADER_STAGES; ++i )
    {
        stagesCreateInfo[i].sType = outShader->shaderStages[i].shaderCreateInfo.sType;
        stagesCreateInfo[i] = outShader->shaderStages[i].stageCreateInfo;
    }

    PipelineDescriptor descriptor = {};

    descriptor.attributes.reserve ( attributeCount );
    std::copy( &attrDescription[0], &attrDescription[attributeCount], std::back_inserter (descriptor.attributes));
    
    descriptor.viewport = viewport;
    descriptor.scissor = scissor;
    descriptor.isWireframe = false;

    descriptor.stages.reserve ( SHADER_STAGES );
    std::copy ( &stagesCreateInfo[0], &stagesCreateInfo[SHADER_STAGES], std::back_inserter ( descriptor.stages ) );

    // descriptor.descriptorSetLayouts  is not being set since we dont have them yet
    if ( !Pipeline::Create ( context, &context->renderPass, descriptor, &outShader->pipeline ) )
    {
        return false;
    }

    return true;
}


bool Shader::Destroy ( VulkanContext* context, Shader* inShader )
{
    Pipeline::Destroy ( context, &inShader->pipeline );

    for ( uint32_t i = 0; i < SHADER_STAGES; ++i )
    {
        vkDestroyShaderModule ( context->logicalDeviceInfo.handle, inShader->shaderStages[i].handle, context->allocator );
        inShader->shaderStages[i] = {};
    }
    return true;
}

bool Shader::Bind ( VulkanContext* context, Shader* inShader )
{
    uint32_t currImage = context->currentImageIndex;
    Pipeline::Bind( &context->swapchainInfo.graphicssCommandBuffers[currImage], VK_PIPELINE_BIND_POINT_GRAPHICS, &inShader->pipeline );
    return true;
}

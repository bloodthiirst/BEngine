#include <Maths/Vector3.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include "Shader.h"
#include "../../BEngine/BEngine/Core/Platform/Base/Filesystem.h"
#include "../../BEngine/BEngine/Core/Logger/Logger.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"
#include "../../../Platform/Base/Memory.h"
#include "../../../Global/Global.h"
#include "../../../Application/Application.h"

bool CreateShaderModule( VulkanContext* context, Filesystem* filesystem, const StringView shaderName, const StringView shaderType, VkShaderStageFlagBits flag, uint32_t stageIndex, ShaderStage* shaderStagesArray )
{
    Allocator alloc = HeapAllocator::Create();

    StringBuffer absolutePath = StringUtils::Concat( alloc, "D:\\Dev\\Projects\\C&C++\\BEngine\\BEngine\\x64\\Debug\\", shaderName, ".", shaderType, ".spv" );
    char* path_as_cStr = StringView::ToCString( absolutePath.view, alloc );

    ShaderStage* shaderStage = &shaderStagesArray[stageIndex];
    memset( shaderStage, 0, sizeof( ShaderStage ) );

    shaderStage->shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // load shader file
    FileHandle fHandle = {};
    if (!filesystem->open( path_as_cStr, FileMode::Read, true, &fHandle ))
    {
        StringBuffer log = StringUtils::Concat( alloc, "Couldn't load shader file in ", path_as_cStr );
        char* log_c_str = StringView::ToCString( log.view, alloc );

        Global::logger.Error( log_c_str );

        StringBuffer::Destroy( &log );
        alloc.free( alloc, log_c_str );

        return false;
    }


    void* dataPtr = {};
    size_t dataSize = 0;
    if (!filesystem->read_all( fHandle, &dataPtr, &dataSize ))
    {
        Global::logger.Error( "Couldn't read file contents" );
        return false;
    }

    shaderStage->shaderCreateInfo.codeSize = dataSize;
    shaderStage->shaderCreateInfo.pCode = (uint32_t*)dataPtr;

    filesystem->close( &fHandle );

    VkResult res = vkCreateShaderModule( context->logicalDeviceInfo.handle, &shaderStage->shaderCreateInfo, context->allocator, &shaderStage->handle );

    if (res != VK_SUCCESS)
    {
        Global::logger.Error( "Shader module creation error" );
        return false;
    }

    shaderStage->stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStage->stageCreateInfo.stage = flag;
    shaderStage->stageCreateInfo.module = shaderStage->handle;
    shaderStage->stageCreateInfo.pName = "main";

    if (dataPtr)
    {
        free( dataPtr );
    }

    return true;

}


bool Shader::Create( VulkanContext* context, Filesystem* filesystem, Shader* outShader )
{
    StringView stageTypesStrings[SHADER_STAGES] = { "vert" , "frag" };

    VkShaderStageFlagBits stageTypes[SHADER_STAGES] = { VK_SHADER_STAGE_VERTEX_BIT , VK_SHADER_STAGE_FRAGMENT_BIT };

    for (uint32_t i = 0; i < SHADER_STAGES; ++i)
    {
        StringView shaderType = stageTypesStrings[i];
        VkShaderStageFlagBits shaderStageBit = stageTypes[i];
        if (!CreateShaderModule( context, filesystem, BUILTIN_SHADER_NAME, shaderType, shaderStageBit, i, outShader->shaderStages ))
        {
            Global::logger.Error( "Create shader mode error" );
            return false;
        }
    }

    // global descriptors
    // binding
    VkDescriptorSetLayoutBinding binding = { };
    binding.binding = 0;
    binding.descriptorCount = 1;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    binding.pImmutableSamplers = nullptr;
    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // layout
    VkDescriptorSetLayoutCreateInfo createLayout = {};
    createLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createLayout.bindingCount = 1;
    createLayout.pBindings = &binding;
    vkCreateDescriptorSetLayout( context->logicalDeviceInfo.handle, &createLayout, context->allocator, &outShader->globalDescriptorSetLayout );

    // pool
    VkDescriptorPoolSize poolSize = {};
    poolSize.descriptorCount = context->swapchain_info.imagesCount;
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolCreateInfo poolCreate = {};
    poolCreate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreate.pPoolSizes = &poolSize;
    poolCreate.poolSizeCount = 1;
    poolCreate.maxSets = context->swapchain_info.imagesCount; // indicates how many descriptor sets are stored in a pool

    vkCreateDescriptorPool( context->logicalDeviceInfo.handle, &poolCreate, context->allocator, &outShader->globalDescriptorPool );

    // create pipeline
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = (float)context->frameBufferSize.y;
    viewport.width = (float)context->frameBufferSize.x;
    viewport.height = (float)-context->frameBufferSize.y;
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
        sizeof( Vector3 )
    };

    for (uint32_t i = 0; i < attributeCount; ++i)
    {
        attrDescription[i].binding = 0;
        attrDescription[i].location = i;
        attrDescription[i].format = formats[i];
        attrDescription[i].offset = offset;

        offset += sizes[i];
    }

    // stages

    VkPipelineShaderStageCreateInfo stagesCreateInfo[SHADER_STAGES];

    for (uint32_t i = 0; i < SHADER_STAGES; ++i)
    {
        stagesCreateInfo[i].sType = outShader->shaderStages[i].shaderCreateInfo.sType;
        stagesCreateInfo[i] = outShader->shaderStages[i].stageCreateInfo;
    }

    Allocator alloc = HeapAllocator::Create();

    PipelineDescriptor descriptor = {};

    DArray<VkVertexInputAttributeDescription>::Create( attributeCount, &descriptor.attributes, alloc );
    descriptor.attributes.size = attributeCount;

    Global::platform.memory.mem_copy( &attrDescription, descriptor.attributes.data, sizeof( VkVertexInputAttributeDescription ) * attributeCount );

    // globa ubo layouts
    const uint32_t layoutsCount = 1;
    DArray< VkDescriptorSetLayout> layouts;
    DArray<VkDescriptorSetLayout>::Create( layoutsCount, &layouts, alloc );
    DArray<VkDescriptorSetLayout>::Add( &layouts, outShader->globalDescriptorSetLayout );

    descriptor.viewport = viewport;
    descriptor.scissor = scissor;
    descriptor.isWireframe = false;
    descriptor.descriptorSetLayouts = layouts;
    DArray< VkPipelineShaderStageCreateInfo>::Create( SHADER_STAGES, &descriptor.stages, alloc );
    descriptor.stages.size = SHADER_STAGES;
    Global::platform.memory.mem_copy( &stagesCreateInfo, descriptor.stages.data, sizeof( VkPipelineShaderStageCreateInfo ) * SHADER_STAGES );

    // descriptor.descriptorSetLayouts  is not being set since we dont have them yet
    if (!Pipeline::Create( context, &context->renderPass, descriptor, &outShader->pipeline ))
    {
        return false;
    }

    // create global ubo buffer
    BufferDescriptor bufferDesc = {};
    bufferDesc.size = sizeof( GlobalUniformObject );
    bufferDesc.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    bufferDesc.usage = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

    if (!Buffer::Create( context, bufferDesc, true, &outShader->globalUBOBuffer ))
    {
        return false;
    }

    // allocate descriptor set layout

    DArray<VkDescriptorSetLayout> layoutsForAlloc;
    DArray<VkDescriptorSetLayout>::Create( context->swapchain_info.imagesCount, &layoutsForAlloc, Global::alloc_toolbox.heap_allocator );
    layoutsForAlloc.size = context->swapchain_info.imagesCount;

    for (size_t i = 0; i < context->swapchain_info.imagesCount; ++i)
    {
        layoutsForAlloc.data[i] = layouts.data[0];
    }

    VkDescriptorSetAllocateInfo allocSets = {};
    allocSets.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocSets.descriptorPool = outShader->globalDescriptorPool;
    allocSets.descriptorSetCount = (uint32_t)layoutsForAlloc.size;
    allocSets.pSetLayouts = layoutsForAlloc.data;
    vkAllocateDescriptorSets( context->logicalDeviceInfo.handle, &allocSets, outShader->globalDescriptorSets );

    return true;
}


bool Shader::Destroy( VulkanContext* context, Shader* inShader )
{
    Buffer::Destroy( context, &inShader->globalUBOBuffer );

    Pipeline::Destroy( context, &inShader->pipeline );

    vkDestroyDescriptorPool( context->logicalDeviceInfo.handle, inShader->globalDescriptorPool, context->allocator );

    vkDestroyDescriptorSetLayout( context->logicalDeviceInfo.handle, inShader->globalDescriptorSetLayout, context->allocator );

    for (uint32_t i = 0; i < SHADER_STAGES; ++i)
    {
        vkDestroyShaderModule( context->logicalDeviceInfo.handle, inShader->shaderStages[i].handle, context->allocator );
        inShader->shaderStages[i] = {};
    }
    return true;
}

bool Shader::UpdateGlobalBuffer( VulkanContext* context, Memory* memroy, GlobalUniformObject globalBuffer, Shader* inShader )
{
    uint32_t currentIndex = context->current_image_index;
    CommandBuffer currentCmdBuffer = context->swapchain_info.graphics_cmd_buffers_per_image.data[currentIndex];
    VkDescriptorSet currentDescriptor = inShader->globalDescriptorSets[currentIndex];


    uint32_t size = sizeof( GlobalUniformObject );
    uint32_t offset = 0;

    if (size > 256)
    {
        Global::logger.Warning( "Global buffer size should be less than 256" );
    }

    Buffer::Load( context, offset, size, &globalBuffer, 0, &inShader->globalUBOBuffer );

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = inShader->globalUBOBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = size;

    VkWriteDescriptorSet writeDesc = {};
    writeDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDesc.descriptorCount = 1;
    writeDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDesc.dstSet = currentDescriptor;
    writeDesc.dstBinding = 0;
    writeDesc.dstArrayElement = 0;
    writeDesc.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets( context->logicalDeviceInfo.handle, 1, &writeDesc, 0, nullptr );

    vkCmdBindDescriptorSets( currentCmdBuffer.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, inShader->pipeline.layout, 0, 1, &currentDescriptor, 0, nullptr );


    return false;
}

bool Shader::Bind( VulkanContext* context, Shader* inShader )
{
    uint32_t currImage = context->current_image_index;
    Pipeline::Bind( &context->swapchain_info.graphics_cmd_buffers_per_image.data[currImage], VK_PIPELINE_BIND_POINT_GRAPHICS, &inShader->pipeline );
    return true;
}

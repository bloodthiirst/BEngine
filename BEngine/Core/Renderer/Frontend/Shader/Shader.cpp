#include "Shader.h"
#include <Maths/Vector3.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include "../../BEngine/BEngine/Core/Platform/Base/Filesystem.h"
#include "../../BEngine/BEngine/Core/Logger/Logger.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"
#include "../../../Renderer/Backend/Vulkan/Renderer/DescriptorManager.h"
#include "../../../Platform/Base/Memory.h"
#include "../../../Global/Global.h"
#include "../../../Application/Application.h"

bool Shader::Destroy( VulkanContext* context, Shader* in_shader )
{
    Buffer::Destroy( context, &in_shader->globalUBOBuffer );

    Pipeline::Destroy( context, &in_shader->pipeline );


    for ( size_t i = 0; i < in_shader->descriptor_set_layouts.size; ++i )
    {
        vkDestroyDescriptorSetLayout( context->logicalDeviceInfo.handle, in_shader->descriptor_set_layouts.data[i], context->allocator);
    }

    DArray<VkDescriptorSetLayout>::Destroy( &in_shader->descriptor_set_layouts );
    in_shader->descriptor_set_layouts = {};

    for ( uint32_t i = 0; i < ShaderStageType::EnumLength; ++i )
    {
        VkShaderModule* shader_mod = &in_shader->shader_modules[i];
        
        if ( *shader_mod == VK_NULL_HANDLE )
            continue;

        vkDestroyShaderModule( context->logicalDeviceInfo.handle, *shader_mod, context->allocator );
        *shader_mod = VK_NULL_HANDLE;
    }

    return true;
}


bool Shader::UpdateGlobalBuffer( VulkanContext* context , GlobalUniformObject globalBuffer, Shader* inShader )
{
    uint32_t currentIndex = context->current_image_index;
    CommandBuffer currentCmdBuffer = context->swapchain_info.graphics_cmd_buffers_per_image.data[currentIndex];
    VkDescriptorSet currentDescriptor = inShader->globalDescriptorSets[currentIndex];

    uint32_t size = sizeof( GlobalUniformObject );
    uint32_t offset = 0;

    if ( size > 256 )
    {
        Global::logger.Warning( "Global buffer size should be less than 256" );
    }

    Buffer::Load( context, offset, size, &globalBuffer, 0, &inShader->globalUBOBuffer );

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = inShader->globalUBOBuffer.handle;
    bufferInfo.offset = offset;
    bufferInfo.range = size;

    VkWriteDescriptorSet writeDescriptor = {};
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = currentDescriptor;   // "dst" stands for "descriptor" here 
    writeDescriptor.dstBinding = 0;               // "dst" stands for "descriptor" herz
    writeDescriptor.dstArrayElement = 0;          // "dst" stands for "descriptor" herz
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptor.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets( context->logicalDeviceInfo.handle, 1, &writeDescriptor, 0, nullptr );

    vkCmdBindDescriptorSets( currentCmdBuffer.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, inShader->pipeline.layout, 0, 1, &currentDescriptor, 0, nullptr );

    return false;
}

bool Shader::Bind( VulkanContext* context, Shader* inShader )
{
    uint32_t currImage = context->current_image_index;
    Pipeline::Bind( &context->swapchain_info.graphics_cmd_buffers_per_image.data[currImage], VK_PIPELINE_BIND_POINT_GRAPHICS, &inShader->pipeline );
    return true;
}

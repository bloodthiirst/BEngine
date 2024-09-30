#include <Maths/Vector3.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include "../../Application/Application.h"
#include "../../Platform/Base/Memory.h"
#include "../../Global/Global.h"
#include "../../Platform/Base/Filesystem.h"
#include "../../Logger/Logger.h"
#include "../Context/VulkanContext.h"
#include "../DescriptorManager/DescriptorManager.h"
#include "Shader.h"

bool Shader::Destroy(VulkanContext *context, Shader *in_shader)
{
    Pipeline::Destroy(context, &in_shader->pipeline);

    for (size_t i = 0; i < in_shader->descriptor_set_layouts.size; ++i)
    {
        vkDestroyDescriptorSetLayout(context->logical_device_info.handle, in_shader->descriptor_set_layouts.data[i], context->allocator);
    }

    DArray<VkDescriptorSetLayout>::Destroy(&in_shader->descriptor_set_layouts);

    for (uint32_t i = 0; i < ShaderStageType::EnumLength; ++i)
    {
        VkShaderModule *shader_mod = &in_shader->shader_modules[i];

        assert(*shader_mod != VK_NULL_HANDLE);

        if (*shader_mod == VK_NULL_HANDLE)
            continue;

        vkDestroyShaderModule(context->logical_device_info.handle, *shader_mod, context->allocator);
        *shader_mod = VK_NULL_HANDLE;
    }

    return true;
}

void Shader::SetBuffer(VulkanContext *context, Shader *in_shader, uint32_t descriptor_set_index, Buffer *in_buffer)
{
    uint32_t current_image = context->current_image_index;

    VkDescriptorSet currentDescriptor = in_shader->descriptor_sets[current_image].data[descriptor_set_index];

    if (in_buffer->descriptor.size < 256)
    {
        Global::logger.Warning("Global buffer size should not be less than 256");
    }

    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = in_buffer->handle;
    bufferInfo.offset = 0;
    bufferInfo.range = in_buffer->descriptor.size;

    VkWriteDescriptorSet writeDescriptor = {};
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = currentDescriptor; // "dst" stands for "descriptor" here
    writeDescriptor.dstBinding = 0;             // "dst" stands for "descriptor" herz
    writeDescriptor.dstArrayElement = 0;        // "dst" stands for "descriptor" herz
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptor.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(context->logical_device_info.handle, 1, &writeDescriptor, 0, nullptr);
}

void Shader::SetTexture(VulkanContext *context, Shader *in_shader, uint32_t descriptor_set_index, Texture *in_texture)
{
    uint32_t current_image = context->current_image_index;

    VkDescriptorSet currentDescriptor = in_shader->descriptor_sets[current_image].data[descriptor_set_index];
    VkDescriptorImageInfo image_info = {};
    image_info.imageView = in_texture->view;
    image_info.sampler = context->default_sampler;
    image_info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeDescriptor = {};
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = currentDescriptor; // "dst" stands for "descriptor" here
    writeDescriptor.dstBinding = 0;             // "dst" stands for "descriptor" here
    writeDescriptor.dstArrayElement = 0;        // "dst" stands for "descriptor" here
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptor.pImageInfo = &image_info;

    vkUpdateDescriptorSets(context->logical_device_info.handle, 1, &writeDescriptor, 0, nullptr);
}

bool Shader::Bind(VulkanContext *context, Shader *in_shader)
{
    uint32_t currImage = context->current_image_index;
    Pipeline::Bind(&context->swapchain_info.graphics_cmd_buffers_per_image.data[currImage], VK_PIPELINE_BIND_POINT_GRAPHICS, &in_shader->pipeline);
    return true;
}

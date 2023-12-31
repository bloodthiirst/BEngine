#include "ShaderBuilder.h"
#include "../../Backend/Vulkan/Context/VulkanContext.h"

ShaderBuilder ShaderBuilder::SetStage( ShaderStageType type, StringView code )
{
    stages_present[type] = true;

    const VkShaderStageFlagBits flags[ShaderStageType::EnumLength] =
    {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    ShaderStage* info = &stage_info[type];

    *info = {};
    info->shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info->shader_create_info.codeSize = code.length;
    info->shader_create_info.pCode = (uint32_t*) code.buffer;

    info->pipeline_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info->pipeline_stage_create_info.stage = flags[type];
    info->pipeline_stage_create_info.module = nullptr;
    info->pipeline_stage_create_info.pName = "main";

    return *this;
}

ShaderBuilder ShaderBuilder::AddImageDescriptor( size_t layout, size_t binding, VkDescriptorType binding_stage )
{
    DescriptorPair desc = {};
    desc.layout = layout;
    desc.binding = binding;
    desc.binding = binding_stage;

    // todo : maybe a good change to implement a dictionary
    DArray<DescriptorPair>::Add( &descriptors, desc );

    return *this;
}

ShaderBuilder ShaderBuilder::AddBufferDescriptor( size_t layout, size_t binding, VkDescriptorType bindingStage )
{
    return *this;
}

bool ShaderBuilder::Build( VulkanContext* context, Shader* out_shader )
{
    *out_shader = {};

    // create shader modules
    {
        for ( size_t i = 0; i < ShaderStageType::EnumLength; ++i )
        {
            if ( !stages_present[i] )
                continue;

            ShaderStage* shader_stage = &stage_info[i];

            VkResult res = vkCreateShaderModule( context->logicalDeviceInfo.handle, &shader_stage->shader_create_info, context->allocator, &shader_stage->handle );

            if ( res != VK_SUCCESS )
            {
                Global::logger.Error( "Shader module creation error" );
                return false;
            }

            out_shader->shader_modules[i] = shader_stage->handle;
        }
    }

    // global descriptors

    {
        // binding = 0 correspond to the binding in the shader
        VkDescriptorSetLayoutBinding binding = { };
        binding.binding = 0;
        binding.descriptorCount = 1;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        // layout
        // correspond to the layout in the shader , also should include the corresponding bindings in the shader as well
        // hence the reason why pBindings points to "binding" that we previously created
        VkDescriptorSetLayoutCreateInfo createLayout = {};
        createLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createLayout.bindingCount = 1;
        createLayout.pBindings = &binding;
        vkCreateDescriptorSetLayout( context->logicalDeviceInfo.handle, &createLayout, context->allocator, &out_shader->globalDescriptorSetLayout );

        // pool
        // will be used to allocate the descriptor set layouts from them
        // notice how the pool needs to specify the type of descriptor it spawns
        // in our case it's only uniform buffers
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = context->swapchain_info.imagesCount;

        // create the pool
        // note that "poolSizeCount" here doesn't refer the amount of descriptors the pool is able to provider
        // it is simple to indicate how many acutal pools we want to create , in this case only 1 pool

        // maxSets on the other hand , indicates the capacity of the pools (which means how many descriptor sets the pool contains)
        VkDescriptorPoolCreateInfo poolCreate = {};
        poolCreate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreate.pPoolSizes = &poolSize;
        poolCreate.poolSizeCount = 1;
        poolCreate.maxSets = context->swapchain_info.imagesCount;

        vkCreateDescriptorPool( context->logicalDeviceInfo.handle, &poolCreate, context->allocator, &out_shader->globalDescriptorPool );
    }

    // create pipeline
    {
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = (float) context->frameBufferSize.y;
        viewport.width = (float) context->frameBufferSize.x;
        viewport.height = (float) -context->frameBufferSize.y;
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

        for ( uint32_t i = 0; i < attributeCount; ++i )
        {
            attrDescription[i].binding = 0;
            attrDescription[i].location = i;
            attrDescription[i].format = formats[i];
            attrDescription[i].offset = offset;

            offset += sizes[i];
        }

        // pipeline stages
        // since the shader module should already be created at this point
        // all we need to do is set the module for the pipeline
        {
            for ( uint32_t i = 0; i < ShaderStageType::EnumLength; ++i )
            {
                if ( !stages_present[i] )
                    continue;

                ShaderStage* curr = &stage_info[i];
                curr->pipeline_stage_create_info.module = curr->handle;
            }
        }

        Allocator alloc = Global::alloc_toolbox.heap_allocator;

        PipelineDescriptor descriptor = {};

        // copy the vertex attributes
        {
            DArray<VkVertexInputAttributeDescription>::Create( attributeCount, &descriptor.attributes, alloc );
            Global::platform.memory.mem_copy( &attrDescription, descriptor.attributes.data, sizeof( VkVertexInputAttributeDescription ) * attributeCount );
            descriptor.attributes.size = attributeCount;
        }

        // copy the global ubo layouts
        {
            const uint32_t layoutsCount = 1;
            DArray<VkDescriptorSetLayout>::Create( layoutsCount, &descriptor.descriptorSetLayouts, alloc );
            DArray<VkDescriptorSetLayout>::Add( &descriptor.descriptorSetLayouts, out_shader->globalDescriptorSetLayout );
        }

        // copy the stages necessary
        {
            DArray<VkPipelineShaderStageCreateInfo>::Create( ShaderStageType::EnumLength, &descriptor.stages, alloc );

            for ( size_t i = 0; i < ShaderStageType::EnumLength; ++i )
            {
                if ( !stages_present[i] )
                    continue;

                DArray<VkPipelineShaderStageCreateInfo>::Add( &descriptor.stages, stage_info[i].pipeline_stage_create_info );
            }
        }

        descriptor.viewport = viewport;
        descriptor.scissor = scissor;
        descriptor.isWireframe = false;

        // create the pipeline
        if ( !Pipeline::Create( context, &context->renderPass, descriptor, &out_shader->pipeline ) )
        {
            return false;
        }
    }

    // create the actual global ubo buffer
    {
        BufferDescriptor bufferDesc = {};
        bufferDesc.size = sizeof( GlobalUniformObject );
        bufferDesc.memoryPropertyFlags = (VkMemoryPropertyFlagBits) (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        bufferDesc.usage = (VkBufferUsageFlagBits) (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        if ( !Buffer::Create( context, bufferDesc, true, &out_shader->globalUBOBuffer ) )
        {
            return false;
        }
    }

    // allocate global descriptor set layout
    {
        // we create a descriptor per swapchain image
        // but they all share the same layout since they describe the same descriptor/resource
        DArray<VkDescriptorSetLayout> layoutsForAlloc;
        DArray<VkDescriptorSetLayout>::Create( context->swapchain_info.imagesCount, &layoutsForAlloc, STACK_ALLOC_ARRAY( VkDescriptorSetLayout, context->swapchain_info.imagesCount ) );
        layoutsForAlloc.size = context->swapchain_info.imagesCount;

        for ( size_t i = 0; i < context->swapchain_info.imagesCount; ++i )
        {
            layoutsForAlloc.data[i] = out_shader->globalDescriptorSetLayout;
        }

        VkDescriptorSetAllocateInfo allocSets = {};
        allocSets.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocSets.descriptorPool = out_shader->globalDescriptorPool;
        allocSets.descriptorSetCount = (uint32_t) layoutsForAlloc.size;
        allocSets.pSetLayouts = layoutsForAlloc.data;
        vkAllocateDescriptorSets( context->logicalDeviceInfo.handle, &allocSets, out_shader->globalDescriptorSets );
    }

    out_shader->builder = *this;
    return true;
}
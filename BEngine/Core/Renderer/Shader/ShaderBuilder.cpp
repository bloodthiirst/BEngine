#include <Containers/ContainerUtils.h>
#include "../Context/VulkanContext.h"
#include "ShaderBuilder.h"

ShaderBuilder ShaderBuilder::SetStage(VkShaderStageFlagBits type, StringBuffer code)
{
    ShaderStage info = {};
    info.code = code;
    info.stage_flagbits = type;
    DArray<ShaderStage>::Add(&this->shader_stages, info);

    return *this;
}

DescriptorLayoutInfo *GetOrAddLayout(ShaderBuilder *in_builder, size_t layout_index)
{
    DArray<DescriptorLayoutInfo> *arr = &in_builder->descriptor_layouts;

    for (size_t i = 0; i < arr->size; ++i)
    {
        DescriptorLayoutInfo *curr = &arr->data[i];

        if (curr->layout_index == layout_index)
        {
            return curr;
        }
    }

    DescriptorLayoutInfo new_layout = {};
    new_layout.layout_index = layout_index;
    DArray<DescriptorBindingInfo>::Create(1, &new_layout.bindings, Global::alloc_toolbox.heap_allocator);

    DArray<DescriptorLayoutInfo>::Add(arr, new_layout);
    DescriptorLayoutInfo *layout = &arr->data[arr->size - 1];

    return layout;
}

ShaderBuilder ShaderBuilder::AddVertexAttribute(StringView name, size_t location, size_t size, VkFormat format)
{
    VertexAttributeInfo desc = {};
    desc.name = name;
    desc.location = location;
    desc.format = format;
    desc.size = size;

    DArray<VertexAttributeInfo>::Add(&vertex_attributes, desc);

    return *this;
}

ShaderBuilder ShaderBuilder::AddDescriptor(StringView name, size_t layout_index, size_t binding_index, VkDescriptorType type, VkShaderStageFlagBits stage_usage)
{
    DescriptorBindingInfo desc = {};
    desc.name = name;
    desc.binding_index = binding_index;
    desc.type = type;
    desc.stage_usage = stage_usage;

    DescriptorLayoutInfo *layout = GetOrAddLayout(this, layout_index);

    for (size_t i = 0; i < layout->bindings.size; ++i)
    {
        DescriptorBindingInfo *curr = &layout->bindings.data[i];

        bool already_exists = Global::platform.memory.mem_compare(curr, &desc, sizeof(DescriptorBindingInfo)) == 0;

        assert(already_exists && "Binding already exists");
    }

    DArray<DescriptorBindingInfo>::Add(&layout->bindings, desc);

    return *this;
}

bool SortDescriptorSet(DescriptorLayoutInfo a, DescriptorLayoutInfo b)
{
    return a.layout_index > b.layout_index;
}

bool SortDescriptorSetBinding(DescriptorBindingInfo a, DescriptorBindingInfo b)
{
    return a.binding_index > b.binding_index;
}

bool ShaderBuilder::Build(VulkanContext *context, Renderpass* in_renderpass, Shader *out_shader)
{
    *out_shader = {};

    // sort descriptors
    {
        // first , sort by layout index
        ContainerUtils::Sort(descriptor_layouts.data, 0, descriptor_layouts.size, SortDescriptorSet);

        // then , sort by binding index
        for (size_t i = 0; i < descriptor_layouts.size; ++i)
        {
            ContainerUtils::Sort(descriptor_layouts.data->bindings.data, 0, descriptor_layouts.data->bindings.size, SortDescriptorSetBinding);
        }
    }

    // create descriptor layouts and bindings
    {
        DArray<VkDescriptorSetLayout>::Create(2, &out_shader->descriptor_set_layouts, Global::alloc_toolbox.heap_allocator);

        for (size_t i = 0; i < this->descriptor_layouts.size; ++i)
        {
            VkDescriptorSetLayout layout = {};
            DescriptorLayoutInfo *info = &descriptor_layouts.data[i];
            ShaderUtils::CreateDescriptorFromInfo(info, &layout);
            DArray<VkDescriptorSetLayout>::Add(&out_shader->descriptor_set_layouts, layout);
        }
    }

    // create pool
    {
        // pools
        // will be used to allocate the descriptor set layouts from them
        // notice how the pool needs to specify the type of descriptor it spawns
        // in our case it's only uniform buffers
        // VkDescriptorPoolSize subPools[] =
        //{
        //    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , context->swapchain_info.imagesCount},
        //    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , context->swapchain_info.imagesCount},
        //};

        // create the pool
        // note that "poolSizeCount" here doesn't refer the amount of descriptors the pool is able to provider
        // it is simply to indicate how many acutal pools we want to create , in this case only 1 pool

        // a very important distinction to make :
        // A descriptor pool CAN provide descriptors for multiple types , meaning that a single descriptor pool can provide descriptors of the types
        // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , VK_DESCRIPTOR_TYPE_STORAGE_BUFFER AND VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER for example
        // you can think of it as a parent bucket that has internal sub-buckets like this
        //
        //  |    |                       |                       |                     |    |
        //  |    |                       |                       |                     |    |
        //  |    |   UniformBuffer : 5   |   StorageBuffer : 12  |   ImageSampler : 20 |    |
        //  |    |_______________________|_______________________|_____________________|    |
        //  |                                                                               |
        //  |                             Descriptor Pool                                   |
        //  |_______________________________________________________________________________|
        // maxSets specifies the maximum number of descriptor sets that may be allocated:

        DArray<DescriptorPoolInfo *>::Create(this->descriptor_layouts.size, &out_shader->descriptor_pools, Global::alloc_toolbox.heap_allocator);

        for (size_t i = 0; i < this->descriptor_layouts.size; ++i)
        {
            DescriptorPoolInfo *pool = {};
            DescriptorManager::Allocate(this->descriptor_layouts.data[i], &context->descriptor_manager, &pool); // this can cause issues since , one resize the addresses can change , maybe pass the index

            DArray<DescriptorPoolInfo *>::Add(&out_shader->descriptor_pools, pool);
        }
    }

    // create pipeline
    ArenaCheckpoint checkpoint = Global::alloc_toolbox.GetArenaCheckpoint();
    Allocator alloc = Global::alloc_toolbox.frame_allocator;

    PipelineDependencies dependencies = {};
    DArray<VkDescriptorSetLayout>::Create(&out_shader->descriptor_set_layouts, &dependencies.descriptor_set_layouts, out_shader->descriptor_set_layouts.size, alloc);
    DArray<PipelineShaderInfo>::Create(0, &dependencies.shader_info, alloc);

    // create shader modules
    {
        for (size_t i = 0; i < shader_stages.size; ++i)
        {
            ShaderStage *shader_stage = &shader_stages.data[i];

            VkShaderModuleCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            info.codeSize = shader_stage->code.length;
            info.pCode = (uint32_t *)shader_stage->code.buffer;

            VkShaderModule shader_handle = {};
            VkResult res = vkCreateShaderModule(context->logicalDeviceInfo.handle, &info, context->allocator, &shader_handle);

            if (res != VK_SUCCESS)
            {
                Global::logger.Error("Shader module creation error");
                return false;
            }

            out_shader->shader_modules[i] = shader_handle;

            PipelineShaderInfo pipeline_info = {};
            pipeline_info.flags = shader_stage->stage_flagbits;
            pipeline_info.handle = shader_handle;
            pipeline_info.function_name = "main";

            DArray<PipelineShaderInfo>::Add(&dependencies.shader_info, pipeline_info);
        }
    }

    // create the pipeline
    if (!Pipeline::Create(context, in_renderpass, &dependencies, this, &out_shader->pipeline))
    {
        Global::alloc_toolbox.ResetArenaOffset(&checkpoint);
        return false;
    }

    // create the actual global ubo buffer
    {
        BufferDescriptor bufferDesc = {};
        bufferDesc.size = sizeof(GlobalUniformObject);
        bufferDesc.memoryPropertyFlags = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        bufferDesc.usage = (VkBufferUsageFlagBits)(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

        if (!Buffer::Create(context, bufferDesc, true, &out_shader->globalUBOBuffer))
        {
            Global::alloc_toolbox.ResetArenaOffset(&checkpoint);
            return false;
        }
    }

    // allocate descriptor sets
    {
        // we create a descriptor per swapchain image
        // but they all share the same layout since they describe the same descriptor/resource
        for (size_t i = 0; i < context->swapchain_info.imagesCount; ++i)
        {
            DArray<VkDescriptorSet> *descriptor_sets = &out_shader->descriptor_sets[i];
            DArray<VkDescriptorSet>::Create(out_shader->descriptor_set_layouts.size, descriptor_sets, Global::alloc_toolbox.heap_allocator);

            // create a DescriptorSet per layout
            for (size_t j = 0; j < out_shader->descriptor_set_layouts.size; ++j)
            {
                VkDescriptorSetLayout *layout = &out_shader->descriptor_set_layouts.data[j];
                DescriptorPoolInfo *pool_info = out_shader->descriptor_pools.data[j];
                VkDescriptorSetAllocateInfo alloc_descriptor = {};

                alloc_descriptor.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                alloc_descriptor.descriptorPool = pool_info->pool_handle;
                alloc_descriptor.descriptorSetCount = 1;
                alloc_descriptor.pSetLayouts = layout;

                VkDescriptorSet descriptor_set = {};
                vkAllocateDescriptorSets(context->logicalDeviceInfo.handle, &alloc_descriptor, &descriptor_set);

                DArray<VkDescriptorSet>::Add(descriptor_sets, descriptor_set);
            }
        }
    }

    out_shader->builder = *this;
    return true;
}
#include "ShaderUtils.h"
#include "../Context/VulkanContext.h"

void ShaderUtils::CreateDescriptorFromInfo( DescriptorLayoutInfo* in_info, VkDescriptorSetLayout* out_layout )
{
    VulkanContext* context = (VulkanContext*) Global::backend_renderer.user_data;

    ArenaCheckpoint arena = Global::alloc_toolbox.GetArenaCheckpoint();
    DArray<VkDescriptorSetLayoutBinding> tmp_bindings = {};
    DArray<VkDescriptorSetLayoutBinding>::Create( 1, &tmp_bindings, Global::alloc_toolbox.frame_allocator );

    for ( size_t j = 0; j < in_info->bindings.size; ++j )
    {
        DescriptorBindingInfo curr_binding = in_info->bindings.data[j];

        // add the binding
        VkDescriptorSetLayoutBinding binding = { };
        binding.binding = (uint32_t) curr_binding.binding_index;
        binding.descriptorCount = 1; // todo : comeback to this when we wanna bind arrays since this specifies how many elements in the array
        binding.descriptorType = curr_binding.type;
        binding.pImmutableSamplers = nullptr;
        binding.stageFlags = curr_binding.stage_usage;

        DArray<VkDescriptorSetLayoutBinding>::Add( &tmp_bindings, binding );
    }

    // layout
    // correspond to the layout in the shader , also should include the corresponding bindings in the shader as well
    // hence the reason why pBindings points to "binding" that we previously created
    VkDescriptorSetLayoutCreateInfo createLayout = {};
    createLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createLayout.bindingCount = (uint32_t) tmp_bindings.size;
    createLayout.pBindings = tmp_bindings.data;

    vkCreateDescriptorSetLayout( context->logical_device_info.handle, &createLayout, context->allocator, out_layout );

    Global::alloc_toolbox.ResetArenaOffset( &arena );
}

size_t  ShaderUtils::DescriptorLayoutHash( DescriptorLayoutInfo descriptor )
{
    size_t val = descriptor.bindings.size;

    for ( size_t i = 0; i < descriptor.bindings.size; ++i )
    {
        DescriptorBindingInfo binding = descriptor.bindings.data[i];

        size_t binding_hash = binding.type ^ binding.binding_index ^ binding.stage_usage;

        val ^= binding_hash;
    }

    return val;
}

bool  ShaderUtils::DescriptorLayoutComparer( DescriptorLayoutInfo a, DescriptorLayoutInfo b )
{
    if ( a.bindings.size != b.bindings.size )
        return false;

    for ( size_t i = 0; i < a.bindings.size; ++i )
    {
        DescriptorBindingInfo binding_a = a.bindings.data[i];
        bool found = false;

        for ( size_t j = 0; j < b.bindings.size; ++j )
        {
            DescriptorBindingInfo binding_b = b.bindings.data[i];

            bool is_equal =
                binding_b.binding_index == binding_a.binding_index &&
                binding_b.type == binding_a.type;

            if ( is_equal )
            {
                found = true;
                break;
            }
        }

        if ( !found )
        {
            return false;
        }
    }

    return true;
}

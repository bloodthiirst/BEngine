#include "DescriptorManager.h"
#include "../Context/VulkanContext.h"
#include "../../../../Global/Global.h"

void DescriptorManager::Create( VulkanContext* context, DescriptorManager* out_manager )
{
    *out_manager = {};
    HMap<DescriptorLayoutInfo, DescriptorPoolInfo>::Create( &out_manager->pools_map, Global::alloc_toolbox.heap_allocator, 1, 10, ShaderUtils::DescriptorLayoutHash, ShaderUtils::DescriptorLayoutComparer );
}

void DescriptorManager::Destroy( DescriptorManager* in_manager )
{
    VulkanContext* context = (VulkanContext*) Global::backend_renderer.user_data;

    for ( size_t i = 0; i < in_manager->pools_map.count; ++i )
    {
        DescriptorPoolInfo pool = in_manager->pools_map.all_values.data[i];

        vkDestroyDescriptorPool( context->logicalDeviceInfo.handle, pool.pool_handle, VK_NULL_HANDLE );
    }

    HMap<DescriptorLayoutInfo, DescriptorPoolInfo>::Destroy( &in_manager->pools_map );

    *in_manager = {};
}


bool DescriptorManager::Allocate( DescriptorLayoutInfo layout, DescriptorManager* in_manager, DescriptorPoolInfo** out_descriptor )
{
    DescriptorPoolInfo* pool_info = {};

    if ( HMap<DescriptorLayoutInfo, DescriptorPoolInfo>::TryGet( &in_manager->pools_map, layout, &pool_info ) )
    {
        *out_descriptor = pool_info;
        return true;
    }

    VulkanContext* context = (VulkanContext*) Global::backend_renderer.user_data;

    DescriptorPoolInfo pool_to_use = {};

    // create the pool
    {
        ArenaCheckpoint arena = Global::alloc_toolbox.GetArenaCheckpoint();
        Allocator alloc = Global::alloc_toolbox.frame_allocator;
        {
            // note that "poolSizeCount" here doesn't refer the amount of descriptors the pool is able to provider
            // it is simply to indicate how many acutal pools we want to create , in this case only 1 pool
            // maxSets on the other hand , indicates the capacity of the pools (which means how many descriptor sets the pool contains)
            DArray<VkDescriptorPoolSize> pool_sizes = {};
            DArray<VkDescriptorPoolSize>::Create( layout.bindings.size, &pool_sizes, alloc );

            for ( size_t i = 0; i < layout.bindings.size; ++i )
            {
                DArray<VkDescriptorPoolSize>::Add( &pool_sizes, { layout.bindings.data[i].type , 1});
            }

            VkDescriptorPoolCreateInfo poolCreate = {};
            poolCreate.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreate.pPoolSizes = pool_sizes.data;
            poolCreate.poolSizeCount = (uint32_t) pool_sizes.size;
            poolCreate.maxSets = context->swapchain_info.imagesCount * (uint32_t) in_manager->init_sets_count;

            VkDescriptorPool pool = {};
            vkCreateDescriptorPool( context->logicalDeviceInfo.handle, &poolCreate, context->allocator, &pool );

            pool_to_use.pool_handle = pool;
            pool_to_use.allocation_count = 0;
            pool_to_use.total_count = context->swapchain_info.imagesCount * in_manager->init_sets_count;

            DescriptorLayoutInfo layout_copy = {};
            layout_copy.layout_index = layout.layout_index;
            DArray<DescriptorBindingInfo>::Create( &layout.bindings, &layout_copy.bindings, layout.bindings.size, Global::alloc_toolbox.heap_allocator );

            size_t insertion_index = {};
            HMap<DescriptorLayoutInfo, DescriptorPoolInfo>::TryAdd( &in_manager->pools_map, layout_copy, pool_to_use, &insertion_index );

            *out_descriptor = &in_manager->pools_map.all_values.data[insertion_index];

            DArray<VkDescriptorPoolSize>::Destroy( &pool_sizes );
        }

        Global::alloc_toolbox.ResetArenaOffset( &arena );
    }

    return true;

}

bool DescriptorManager::Reset( VulkanContext* context, DescriptorManager* in_manager )
{
    *in_manager = {};

    return true;
}
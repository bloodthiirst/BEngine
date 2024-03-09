#pragma once
#include <vulkan/vulkan.h>
#include <Containers/HMap.h>
#include "../Shader/ShaderUtils.h"
#include "../Shader/ShaderBuilder.h"

struct VulkanContext;

struct DescriptorPoolInfo
{
    VkDescriptorPool pool_handle;
    size_t total_count;
    size_t allocation_count;
};

struct DescriptorManager
{
    size_t init_sets_count;
    float resize_factor;
    HMap<DescriptorLayoutInfo, DescriptorPoolInfo> pools_map;

    static void Create( VulkanContext* context, DescriptorManager* out_manager );

    static void Destroy( DescriptorManager* in_manager );

    static bool Allocate( DescriptorLayoutInfo layout, DescriptorManager* in_manager, DescriptorPoolInfo** out_descriptor );

    static bool Reset( VulkanContext* context, DescriptorManager* in_manager );
};
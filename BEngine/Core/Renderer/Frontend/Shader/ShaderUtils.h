#pragma once
#include <vulkan/vulkan.h>
#include <Containers/DArray.h>
#include "../../../Global/Global.h"
#include "ShaderBuilder.h"

class ShaderUtils
{
public:

    static void CreateDescriptorFromInfo( DescriptorLayoutInfo* in_info, VkDescriptorSetLayout* out_layout );

    static size_t DescriptorLayoutHash( DescriptorLayoutInfo descriptor );

    static bool DescriptorLayoutComparer( DescriptorLayoutInfo a, DescriptorLayoutInfo b );
};
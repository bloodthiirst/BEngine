#pragma once
#include "../../../../Logger/Logger.h"
#include "../../BackendRenderer.h"
#include "../../../Context/RendererContext.h"
#include "../Context/VulkanContext.h"
#include <vulkan/vulkan.h>

struct VulkanBackendRenderer
{
    static void Create(BackendRenderer* out_backend);
};


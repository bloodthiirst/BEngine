#pragma once
#include <vulkan/vulkan.h>

struct VulkanContext;

/// <summary>
/// Fences are used to synchronize between GPU and CPU events , unlike Semaphores that are used to define order between GPU to GPU tasks
/// </summary>
class Fence
{
public:
    VkFence handle;
    bool isSignaled;

public:
    static void Create ( VulkanContext* context, bool isSignaled, Fence* outFance );
    static void Destroy ( VulkanContext* context, Fence* outFance );

    bool Wait ( VulkanContext* context, uint64_t timeoutMs );

    void Reset ( VulkanContext* context );

};


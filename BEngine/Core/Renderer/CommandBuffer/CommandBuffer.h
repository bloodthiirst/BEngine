#pragma once
#include <vulkan/vulkan.h>

enum class CommandBufferState
{
    Ready,
    Recording,
    InRenderpass,
    RecordingEnded,
    Submitted,
    NoAllocated
};


struct VulkanContext;

struct CommandBuffer
{
public:
    VkCommandBuffer handle;
    CommandBufferState state;

public:
    void Begin ( bool isSingleUse, bool isRenderpassContinue, bool isSimultanious );
    void UpdateSubmitted ();
    void Reset ();
    void End ();
    static void Allocate ( VulkanContext* context, VkCommandPool pool, bool isPrimary, CommandBuffer* outCommandBuffer );
    static void Free ( VulkanContext* context, VkCommandPool pool, CommandBuffer* outCommandBuffer );
    static void SingleUseAllocateBegin ( VulkanContext* context, VkCommandPool pool, CommandBuffer* outCommandBuffer );
    static void SingleUseEndSubmit ( VulkanContext* context, VkCommandPool pool, CommandBuffer* outCommandBuffer, VkQueue queue );
};

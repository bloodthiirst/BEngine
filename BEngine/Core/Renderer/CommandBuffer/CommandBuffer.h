#pragma once
#include <vulkan/vulkan.h>
#include "../../Defines/Defines.h"

enum class CommandBufferState
{
    Ready,
    Recording,
    InRenderpass,
    RecordingEnded,
    Submitted,
    NoAllocated
};

struct BAPI CommandBuffer
{
    VkCommandBuffer handle;
    CommandBufferState state;

    void Begin ( bool isSingleUse, bool isRenderpassContinue, bool isSimultanious );
    void UpdateSubmitted ();
    void Reset ();
    void End ();
    static void Allocate (VkCommandPool pool, bool isPrimary, CommandBuffer* out_command_buffer );
    static void Free (VkCommandPool pool, CommandBuffer* out_command_buffer );
    static void SingleUseAllocateBegin (VkCommandPool pool, CommandBuffer* out_command_buffer );
    static void SingleUseEndSubmit (VkCommandPool pool, CommandBuffer* out_command_buffer, VkQueue queue );
};

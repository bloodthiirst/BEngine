#include "../../Renderer/Buffer/Buffer.h"
#include "../../Renderer/Context/VulkanContext.h"
#include "../../Global/Global.h"
#include "Mesh3D.h"
#include "Vertex3D.h"

void Mesh3D::Create(Mesh3D *out_mesh, ArrayView<Vertex3D> verts, ArrayView<uint32_t> indicies, Allocator alloc)
{
    *out_mesh = {};
    out_mesh->alloc = alloc;

    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
    VkCommandPool pool = ctx->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool;
    VkQueue queue = ctx->physicalDeviceInfo.queuesInfo.graphicsQueue;

    DArray<Vertex3D>::Create(verts.data, &out_mesh->vertices, verts.size, out_mesh->alloc);
    DArray<uint32_t>::Create(indicies.data, &out_mesh->indicies, indicies.size, out_mesh->alloc);

    const uint32_t verts_size = sizeof(Vertex3D) * verts.size;
    const uint32_t index_size = sizeof(uint32_t) * indicies.size;

    FreeList::AllocBlock(&ctx->mesh_freelist, verts_size, &out_mesh->verticies_block);
    FreeList::AllocBlock(&ctx->mesh_freelist, index_size, &out_mesh->indicies_block);

    UploadDataRange(ctx, pool, {}, queue, &ctx->mesh_buffer, out_mesh->verticies_block.start, out_mesh->verticies_block.size, verts.data);
    UploadDataRange(ctx, pool, {}, queue, &ctx->mesh_buffer, out_mesh->indicies_block.start, out_mesh->indicies_block.size, indicies.data);
}

void Mesh3D::Destroy(Mesh3D *inout_mesh)
{
    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

    FreeList::FreeBlock(&ctx->mesh_freelist, inout_mesh->verticies_block);
    FreeList::FreeBlock(&ctx->mesh_freelist, inout_mesh->indicies_block);

    DArray<Vertex3D>::Destroy(&inout_mesh->vertices);
    DArray<uint32_t>::Destroy(&inout_mesh->indicies);

    *inout_mesh = {};
}

bool Mesh3D::UploadDataRange(VulkanContext *context, VkCommandPool pool, Fence fence, VkQueue queue, Buffer *in_buffer, uint32_t offset, uint32_t size, void* in_data)
{
    // first , we create a host-visible staging buffer to upload the data to in
    // then we mark it as the source of the transfer
    VkMemoryPropertyFlagBits usage = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Buffer staging_buffer = {};

    BufferDescriptor buffer_desc = {};
    buffer_desc.size = size;
    buffer_desc.memoryPropertyFlags = usage;
    buffer_desc.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Buffer::Create(buffer_desc, true, &staging_buffer);

    Buffer::Load(0, size, in_data, 0, &staging_buffer);

    Buffer::Copy(pool, fence, queue, &staging_buffer, 0, in_buffer, offset, size);

    Buffer::Destroy(&staging_buffer);

    return true;
}
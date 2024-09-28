#include "../../Renderer/Buffer/Buffer.h"
#include "../../Renderer/Context/VulkanContext.h"
#include "../../Global/Global.h"
#include <Containers/DArray.h>
#include "Mesh3D.h"
#include "Vertex3D.h"

void Mesh3D::Create(Mesh3D *out_mesh, ArrayView<Vertex3D> verts, ArrayView<uint32_t> indicies, Allocator alloc)
{
    *out_mesh = {};
    out_mesh->alloc = alloc;

    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;
    VkCommandPool pool = ctx->physical_device_info.command_pools_info.graphicsCommandPool;
    VkQueue queue = ctx->physical_device_info.queues_info.graphics_queue;

    DArray<Vertex3D>::Create(verts.size, &out_mesh->vertices, out_mesh->alloc);
    DArray<uint32_t>::Create(indicies.size, &out_mesh->indicies, out_mesh->alloc);

    AllocData(out_mesh , verts , indicies);
}

void Mesh3D::AllocData(Mesh3D* inout_mesh, ArrayView<Vertex3D> verts, ArrayView<uint32_t> indicies)
{
    assert(inout_mesh->vertices.size == 0);
    assert(inout_mesh->indicies.size == 0);
    assert(inout_mesh->verticies_block.size == 0 && inout_mesh->verticies_block.start == 0);
    assert(inout_mesh->indicies_block.size == 0 && inout_mesh->indicies_block.start == 0);

    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

    const uint32_t verts_size = sizeof(Vertex3D) * verts.size;
    const uint32_t index_size = sizeof(uint32_t) * indicies.size;

    FreeList::AllocBlock(&ctx->mesh_freelist, verts_size, &inout_mesh->verticies_block);
    FreeList::AllocBlock(&ctx->mesh_freelist, index_size, &inout_mesh->indicies_block);

    // first , we create a host-visible staging buffer to upload the data to in
    // then we mark it as the source of the transfer
    VkMemoryPropertyFlagBits usage = (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Buffer staging_buffer = {};

    BufferDescriptor buffer_desc = {};
    buffer_desc.size = Maths::Max(verts_size , index_size);
    buffer_desc.memoryPropertyFlags = usage;
    buffer_desc.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VkCommandPool pool = ctx->physical_device_info.command_pools_info.graphicsCommandPool;
    VkQueue queue = ctx->physical_device_info.queues_info.graphics_queue;

    Buffer::Create(buffer_desc, true, &staging_buffer);

    // copy verts
    {
        Buffer::Load(0, verts_size, verts.data, 0, &staging_buffer);
        Buffer::Copy( pool, {}, queue, &staging_buffer, 0, &ctx->mesh_buffer, inout_mesh->verticies_block.start, verts_size);
    }

    // copy indicies
    {
        Buffer::Load(0, index_size, indicies.data, 0, &staging_buffer);
        Buffer::Copy( pool, {}, queue, &staging_buffer, 0, &ctx->mesh_buffer, inout_mesh->indicies_block.start, index_size);
    }

    Buffer::Destroy(&staging_buffer);
}

void Mesh3D::FreeData(Mesh3D* inout_mesh)
{
    VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

    DArray<Vertex3D>::Clear(&inout_mesh->vertices);
    DArray<uint32_t>::Clear(&inout_mesh->indicies);

    FreeList::FreeBlock(&ctx->mesh_freelist, inout_mesh->indicies_block);
    FreeList::FreeBlock(&ctx->mesh_freelist, inout_mesh->verticies_block);

    inout_mesh->indicies_block = {};
    inout_mesh->verticies_block = {};
}

void Mesh3D::Destroy(Mesh3D *inout_mesh)
{
    FreeData(inout_mesh);
    
    DArray<Vertex3D>::Destroy(&inout_mesh->vertices);
    DArray<uint32_t>::Destroy(&inout_mesh->indicies);

    *inout_mesh = {};
}
#pragma once
#include <Allocators/Allocator.h>
#include <Containers/DArray.h>
#include <Containers/FreeList.h>
#include <Containers/ArrayView.h>
#include <Maths/Vector3.h>
#include "../../Defines/Defines.h"
#include "Vertex3D.h"

struct BAPI Mesh3D
{
    FreeList::Node verticies_block;
    FreeList::Node indicies_block;
    DArray<Vertex3D> vertices;
    DArray<uint32_t> indicies;
    Allocator alloc;

    static void Create(Mesh3D *out_mesh, ArrayView<Vertex3D> verts, ArrayView<uint32_t> indicies, Allocator alloc);
    static void Destroy(Mesh3D *inout_mesh);

private:
    static bool UploadDataRange(VulkanContext *context, VkCommandPool pool, Fence fence, VkQueue queue, Buffer *in_buffer, uint32_t offset, uint32_t size, void *in_data);
};
#pragma once
#include "GlobalAssetManager.h"
#include "../Defines/Defines.h"
#include "../Renderer/Shader/ShaderBuilder.h"

struct BAPI MeshAssetManager
{
    static inline const char *ASSET_ID = "MESH_ASSET_ID";

    static inline size_t ids = 0;

    static void Create(AssetManager *out_manager)
    {
        *out_manager = {};
        out_manager->name = "Mesh Asset Manager";
        out_manager->id = ASSET_ID;
        out_manager->can_import = CanImportMesh;
        out_manager->import = ImportMesh;
        out_manager->free = FreeMesh;
        DArray<AssetHandle>::Create(2, &out_manager->handles, Global::alloc_toolbox.heap_allocator);
    }

private:
    static bool FreeMesh(AssetManager *in_manager, AssetHandle *inout_handle)
    {
        FREE(Global::alloc_toolbox.heap_allocator, inout_handle->data);
        return true;
    }

    static bool CanImportMesh(AssetManager *in_manager, ArrayView<char> data)
    {
        return true;
    }

    static bool ImportMesh(AssetManager *in_manager, ArrayView<char> asset_data, FileHandle file_handle, AssetHandle *out_handle)
    {
        *out_handle = {};

        // assign id
        out_handle->type_id = ASSET_ID;

        // fill info
        out_handle->info.size = asset_data.size;
        out_handle->info.name = StringBuffer::Create("MESH_ASSET_PLACEHOLDER", Global::alloc_toolbox.heap_allocator);
        out_handle->info.asset_id = 0;
        out_handle->file_handle = file_handle;

        // copy data
        out_handle->data = (void *)ALLOC(Global::alloc_toolbox.heap_allocator, asset_data.size);
        Global::platform.memory.mem_copy(asset_data.data, out_handle->data, asset_data.size);

        return true;
    }
};

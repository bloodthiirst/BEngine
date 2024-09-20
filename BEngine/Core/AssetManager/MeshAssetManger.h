#pragma once
#include "GlobalAssetManager.h"
#include "../Defines/Defines.h"

struct BAPI MeshAssetManager
{
    static void Create(AssetManager *out_manager)
    {
        *out_manager = {};
        out_manager->can_import = CanImportMesh;
        out_manager->import = ImportMesh;
        out_manager->free = FreeMesh;
    }

    static bool FreeMesh(AssetHandle inout_handle)
    {
        FREE(Global::alloc_toolbox.heap_allocator , inout_handle.data);
        return true;
    }


    static bool CanImportMesh(ArrayView<char> data)
    {
        return true;
    }

    static bool ImportMesh(ArrayView<char> asset_data, FileHandle file_handle, AssetHandle *out_handle)
    {
        *out_handle = {};

        // assign ID
        {
            char id[] = "MESH_ASSET_ID";
            Global::platform.memory.mem_copy((void*)&id, (void*)&out_handle->metadata, sizeof(id));
        }

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

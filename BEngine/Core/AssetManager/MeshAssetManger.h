#pragma once
#include "GlobalAssetManager.h"
#include "../Defines/Defines.h"
#include "../Global/Global.h"
#include "../Renderer/Mesh/Mesh3D.h"

struct BAPI MeshAssetManager
{
    static inline const char *ASSET_ID = "MESH_ASSET_ID";

    static inline const char *MANAGER_NAME = "Mesh Asset Manager";

    static inline size_t ids = 0;

    static void Create(AssetManager *out_manager)
    {
        *out_manager = {};
        out_manager->name = MANAGER_NAME;
        out_manager->id = ASSET_ID;
        out_manager->can_import = CanImportMesh;
        out_manager->import = ImportMesh;
        out_manager->free = FreeMesh;
        DArray<AssetHandle>::Create(2, &out_manager->handles, Global::alloc_toolbox.heap_allocator);
    }
    
    static bool Unimport(AssetManager *in_manager, AssetHandle* inout_handle)
    {
        return in_manager->free(in_manager , inout_handle);
    }

    static bool Import(AssetManager *in_manager, Mesh3D mesh, FileHandle in_handle, AssetHandle *out_handle)
    {
        ArrayView<char> view = {};
        view.data = (char*) &mesh;
        view.size = sizeof(mesh);

        return in_manager->import(in_manager, view, in_handle, out_handle);
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

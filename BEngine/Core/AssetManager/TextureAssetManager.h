#pragma once
#include <String/StringView.h>
#include "../Defines/Defines.h"
#include "GlobalAssetManager.h"
#include "../Renderer/Texture/Texture.h"

struct BAPI TextureAssetManager
{
    static inline const char *ASSET_ID = "Texture_ASSET_ID";

    static inline const char *MANAGER_NAME = "Texture Asset Manager";

    static inline size_t ids = 0;

    static void Create(AssetManager *out_manager)
    {
        *out_manager = {};
        out_manager->name = MANAGER_NAME;
        out_manager->id = ASSET_ID;
        out_manager->can_import = CanImportTexture;
        out_manager->import = ImportTexture;
        out_manager->free = FreeTexture;
        DArray<AssetHandle>::Create(2, &out_manager->handles, Global::alloc_toolbox.heap_allocator);
    }

    static bool Unimport(AssetManager *in_manager, AssetHandle* inout_handle)
    {
        return in_manager->free(in_manager , inout_handle);
    }

    static bool Import(AssetManager *in_manager, Texture texture, FileHandle in_handle, AssetHandle *out_handle)
    {
        ArrayView<char> view = {};
        view.data = (char*) &texture;
        view.size = sizeof(texture);

        return in_manager->import(in_manager, view, in_handle, out_handle);
    }

private:
    static bool FreeTexture(AssetManager *in_manager,AssetHandle* inout_handle)
    {
        FREE(Global::alloc_toolbox.heap_allocator, inout_handle->data);
        *inout_handle = {};
        return true;
    }

    static bool CanImportTexture(AssetManager *in_manager,ArrayView<char> data)
    {
        return true;
    }

    static bool ImportTexture(AssetManager *in_manager,ArrayView<char> asset_data, FileHandle file_handle, AssetHandle* out_handle)
    {
        void* copy = ALLOC(Global::alloc_toolbox.heap_allocator , asset_data.size);
        Global::platform.memory.mem_copy(asset_data.data, copy, asset_data.size);

        *out_handle = {};
        out_handle->type_id = ASSET_ID;
        out_handle->file_handle;
        out_handle->data = copy;
        out_handle->info.asset_id = ids++;
        out_handle->info.name = StringBuffer::Create("TEXTURE_ASSET_PLACEHOLDER", Global::alloc_toolbox.heap_allocator);
        out_handle->info.size = asset_data.size;

        DArray<AssetHandle>::Add(&in_manager->handles, *out_handle);

        return true;
    }
};

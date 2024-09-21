#pragma once
#include <String/StringBuffer.h>
#include <Typedefs/Typedefs.h>
#include <Containers/ArrayView.h>
#include <Containers/DArray.h>
#include "../Defines/Defines.h"
#include "../Platform/Base/Filesystem.h"

struct BAPI AssetInfo
{
    StringBuffer name;
    uint32_t size;
    uint32_t asset_id;
};

struct BAPI AssetHandle
{
    const char* type_id;
    AssetInfo info;
    FileHandle file_handle;
    void* data;
};

struct BAPI AssetManager
{
    StringView name;
    const char* id;
    DArray<AssetHandle> handles;
    Func<bool, AssetManager*,ArrayView<char>> can_import;
    Func<bool, AssetManager*,ArrayView<char>, FileHandle, AssetHandle*> import;
    Func<bool, AssetManager*,AssetHandle*> free; 
};

struct BAPI GlobalAssetManager
{
    DArray<AssetManager> asset_managers;

    void Startup();

    bool GetByID(const char* id, AssetManager* out_manager) const;

    void Destroy();
};
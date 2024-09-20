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
    char metadata[256];
    AssetInfo info;
    FileHandle file_handle;
    void* data;
};

struct BAPI AssetManager
{
    StringBuffer name;
    DArray<AssetHandle> handles;
    Func<bool, ArrayView<char>> can_import;
    Func<bool, ArrayView<char>, FileHandle, AssetHandle*> import;
    Func<bool, AssetHandle> free; 
};

struct BAPI GlobalAssetManager
{
    DArray<AssetManager> asset_managers;

    void Startup();

    void Destroy();
};
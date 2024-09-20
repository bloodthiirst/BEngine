#include "GlobalAssetManager.h"
#include "../../Core/Global/Global.h"

void GlobalAssetManager::Startup()
{
    DArray<AssetManager>::Create(2, &asset_managers, Global::alloc_toolbox.heap_allocator, true);
}

void GlobalAssetManager::Destroy()
{
    DArray<AssetManager>::Destroy(&asset_managers);
    *this = {};
}
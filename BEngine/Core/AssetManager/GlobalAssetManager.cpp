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

bool GlobalAssetManager::GetByID(const char* id, AssetManager* out_manager) const
{
    for(size_t i = 0; i < asset_managers.size ; ++i)
    {
        AssetManager* curr = &asset_managers.data[i];

        if(curr->id != id)
        {
            continue;
        }

        *out_manager = *curr;
        
        return true;
    }

    return false;
}
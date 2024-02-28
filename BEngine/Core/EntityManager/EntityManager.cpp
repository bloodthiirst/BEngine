#include <GameApp.h>
#include "EntityManager.h"
#include "../Defines/Defines.h"
#include "../Global/Global.h"
#include "../Platform/Base/Platform.h"


void EntityManager::Create( EntityManager* entity_manager )
{
    DArray<Entity>::Create( 1, &entity_manager->entites, Global::alloc_toolbox.heap_allocator);
}

void EntityManager::Destroy( EntityManager* entity_manager )
{
    DArray<Entity>::Destroy( &entity_manager->entites);  
}


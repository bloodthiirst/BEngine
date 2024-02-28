#pragma once
#include <Containers/DArray.h>
#include "../ApplicationState/ApplicationState.h"

struct Entity
{
    size_t id;
    Action on_create;
    Action on_update;
    Action on_destroy;
};

struct EntityManager
{
public:
    DArray<Entity> entites;

    static void Create( EntityManager* entity_manager );
    static void Destroy( EntityManager* entity_manager );

    static void AddEntity( EntityManager* manager, Entity* entity )
    {
        entity->id = manager->entites.size;
        DArray<Entity>::Add( &manager->entites, *entity );
    }

    static void RemoveEntity( EntityManager* manager, Entity* entity )
    {
        for ( size_t i = 0; i < manager->entites.size; ++i )
        {
            if ( manager->entites.data[i].id != entity->id )
                continue;

            DArray<Entity>::RemoveAt( &manager->entites, i );
            return;
        }
    }
};
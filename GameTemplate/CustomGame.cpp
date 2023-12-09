#include "CustomGame.h"
#include "Core/Global/Global.h"
#include "Core/Logger/Logger.h"

static const char* game_name = "Custom game title";

StringView GetName(GameApp* game_app)
{
    return game_name;
}

void Initialize( GameApp* game_app )
{
    
}

void OnUpdate( GameApp* game_app, float delta_time )
{
   // Global::logger.Log("On tick");
}

void OnRender( GameApp* game_app, float delta_time )
{

}

void Destroy( GameApp* game_app )
{

}

extern "C" __declspec(dllexport) GameApp GetGameApp()
{
    GameApp game = {};

    game.get_name = GetName;
    game.initialize = Initialize;
    game.on_update = OnUpdate;
    game.on_render = OnRender;
    game.destroy = Destroy;

    game.game_startup.x = 250;
    game.game_startup.y = 300;
    game.game_startup.width = 500;
    game.game_startup.height = 500;

    return game;
}

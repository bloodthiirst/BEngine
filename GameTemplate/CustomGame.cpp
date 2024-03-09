#include "CustomGame.h"
#include <String/StringBuffer.h>
#include <Typedefs/Typedefs.h>
#include "Core/Global/Global.h"
#include "Core/Logger/Logger.h"
#include <Maths/Maths.h>
#include "SceneCameraController.h"

static const char* game_name = "Custom game title";

struct CustomGameState
{
    SceneCameraController camera_controller;
};

StringView GetName( GameApp* game_app )
{
    return game_name;
}

void Initialize( GameApp* game_app )
{
    game_app->game_state.camera_position = Vector3( 0, 0, -3 );
    game_app->game_state.camera_rotation = { 1,0,0,0 };

    CustomGameState* state = Global::alloc_toolbox.HeapAlloc<CustomGameState>();
    state->camera_controller.position = Vector3( 0, 0, -3 );
    state->camera_controller.xRotation = 0;
    state->camera_controller.yRotation = 0;

    game_app->user_data = state;
}

void OnUpdate( GameApp* game_app, float delta_time )
{
    CustomGameState* state = (CustomGameState*) game_app->user_data;

    Vector3 new_pos = {};
    Quaternion new_rot = {};

    state->camera_controller.Tick( delta_time, &new_pos, &new_rot );

    game_app->game_state.camera_position = new_pos;
    game_app->game_state.camera_rotation = new_rot;


    // TODO : investigate arena init per module
    //Global::logger.Log( StringUtils::Format( Global::alloc_toolbox.frame_allocator, "Current camera pos {} , {} , {}", old_pos->x, old_pos->y, old_pos->z ).view );
}

void OnRender( GameApp* game_app, float delta_time )
{

}

void Destroy( GameApp* game_app )
{
    Global::alloc_toolbox.HeapFree( (CustomGameState*) game_app->user_data );
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

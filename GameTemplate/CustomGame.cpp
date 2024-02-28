#include "CustomGame.h"
#include "Core/Global/Global.h"
#include "Core/Logger/Logger.h"
#include <Maths/Maths.h>

static const char* game_name = "Custom game title";

StringView GetName( GameApp* game_app )
{
    return game_name;
}

void Initialize( GameApp* game_app )
{
    game_app->game_state.camera_position = Vector3( 0, 0, -1 );
    game_app->game_state.camera_rotation = Vector3::Zero();
}

void OnUpdate( GameApp* game_app, float delta_time )
{

    Vector3 delta = { 0,0,0 };

    if ( Global::platform.input.IsDown( KeyCode::KEY_RIGHT ) )
    {
        delta.x += 1;
    }

    if ( Global::platform.input.IsDown( KeyCode::KEY_LEFT ) )
    {
        delta.x -= 1;
    }

    if ( Global::platform.input.IsDown( KeyCode::KEY_UP ) )
    {
        delta.y += 1;
    }

    if ( Global::platform.input.IsDown( KeyCode::KEY_DOWN ) )
    {
        delta.y -= 1;
    }

    if ( delta != Vector3::Zero() )
    {
        delta = Vector3::Normalize( delta );

        Vector3 old_pos = game_app->game_state.camera_rotation;
        Vector3 new_pos = old_pos + delta;

        new_pos.x = Maths::ClampAngle( new_pos.x );
        new_pos.y = Maths::ClampAngle( new_pos.y );
        new_pos.z = Maths::ClampAngle( new_pos.z );

        game_app->game_state.camera_rotation = new_pos;


        // TODO : investigate arena init per module
        //Global::logger.Log( StringUtils::Format( Global::alloc_toolbox.frame_allocator, "Current camera pos {} , {} , {}", old_pos->x, old_pos->y, old_pos->z ).view );
    }
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

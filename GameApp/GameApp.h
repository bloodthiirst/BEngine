#pragma once
#include <String/StringBuffer.h>
#include <Typedefs/Typedefs.h>
#include <Maths/Vector3.h>
#include <Maths/Quaternion.h>

struct GameStartup
{
    int width;
    int height;
    int x;
    int y;
};

struct GameState
{
    Vector3 camera_position;
    Quaternion camera_rotation;
    bool is_running;
};

struct GameApp
{
    Func<StringView, GameApp*> get_name;
    ActionParams<GameApp*> initialize;
    ActionParams<GameApp*, float> on_update;
    ActionParams<GameApp*, float> on_render;
    ActionParams<GameApp*> destroy;

    GameStartup game_startup = { 0 };
    GameState game_state = { };
    void* user_data;
};
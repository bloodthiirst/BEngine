#pragma once
#include <String/StringBuffer.h>
#include <Typedefs/Typedefs.h>
struct __declspec(dllexport) GameStartup
{
public:
    int width;
    int height;
    int x;
    int y;
};


struct __declspec(dllexport) GameState
{
public:
    bool isRunning;

};

struct GameApp
{
    Func<StringView, GameApp*> get_name;
    ActionParams<GameApp*> initialize;
    ActionParams<GameApp*, float> on_update;
    ActionParams<GameApp*, float> on_render;
    ActionParams<GameApp*> destroy;

    GameStartup game_startup = { 0 };
    GameState game_state = { 0 };
};
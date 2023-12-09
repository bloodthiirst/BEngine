#pragma once
#include <String/StringView.h>
#include <Maths/Rect.h>
#include <GameApp.h>
#include "../ApplicationState/ApplicationState.h"

struct GameApp;

struct ApplicationStartup
{
    StringView executable_folder;
    StringView executable_name;
    Rect window_rect;
};

struct Application
{
	GameApp game_app;
	ApplicationState application_state;

	bool Run ();	
};


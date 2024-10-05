#pragma once
#include <String/StringView.h>
#include <Maths/Rect.h>
#include <GameApp.h>
#include "../ApplicationState/ApplicationState.h"


struct ApplicationStartup
{
    StringView executable_folder;
    StringView executable_name;
    Rect window_rect;
};

struct Application
{
	GameApp game_app;
	ApplicationStartup application_startup;
    ApplicationState application_state;


	bool Run ();	
};


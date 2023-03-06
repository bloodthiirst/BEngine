#pragma once
#include "../ApplicationState/ApplicationState.h"
#include "../EventSystem/GameEventSystem.h"
#include "../Platform/Base/Platform/Platform.h"
#include "../Time/Time.h"
#include "../Renderer/Frontend/FrontendRenderer.h"

#include <GameApp.h>


class Application
{
public:
	Application( GameApp* gameApp );

public:
	GameApp* gameApp;
	ApplicationState applicationState;
	GameEventSystem gameEventSystem;
	Platform* platform;
	Time* time;
	FrontendRenderer renderer;

public:
	void Startup ();
	bool Run ();	
	void Destroy ();

	~Application ();
};


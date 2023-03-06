#include "Application.h"

#ifdef _WIN32
#include "../Platform/Types/Win32/Platform/Win32Platform.h"
#endif

#include "../Renderer/Backend/BackendRenderer.h"
#include "../Renderer/Backend/Vulkan/Renderer/VulkanBackendRenderer.h"
#include <format>

Application::Application ( GameApp* gameApp )
{
	this->gameApp = gameApp;

#ifdef _WIN32
	this->platform = new Win32Platform ( this );
#endif 

	this->applicationState = ApplicationState ();
	this->gameEventSystem = GameEventSystem ();
	this->time = new Time ( this );

	BackendRenderer* backend = new VulkanBackendRenderer ( this, this->platform );
	this->renderer = FrontendRenderer ( this, this->platform, backend );
}

void Application::Startup ()
{
	this->gameEventSystem.Startup ();
	this->platform->Startup ();
	this->renderer.Startup ();
}

bool Application::Run ()
{
	applicationState.isRunning = true;

	while ( applicationState.isRunning = platform->window->HandleMessages () )
	{
		platform->input->OnUpdate ( 0 );

		gameApp->OnUpdate (0);

		//gameApp->OnRender ();
		RendererContext rendererCtx = RendererContext ();
		renderer.DrawFrame ( rendererCtx );

		platform->input->OnPostUpdate ( 0 );
	}

	return true;
}

void Application::Destroy ()
{
	this->renderer.Destroy ();
	this->platform->Destroy ();
	this->time->Destroy ();
	this->gameEventSystem.Destroy ();
}



Application::~Application ()
{
	delete time;
	delete platform;
}


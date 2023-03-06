#pragma once
#include "../../Platform/Base/Platform/Platform.h"
#include "../Context/RendererContext.h"

class BackendRenderer;
class Application;

class FrontendRenderer
{
private:
	Application* application;
	Platform* platform;
	BackendRenderer* backend;

public:
	FrontendRenderer () = default;
	FrontendRenderer ( Application* application, Platform* platform, BackendRenderer* backend );
	bool Startup ();

	bool StartFrame ( RendererContext rendererContext );
	bool EndFrame ( RendererContext rendererContext );
	void Resize ( uint32_t width, uint32_t height );
	bool DrawFrame ( RendererContext rendererContext );

	void Destroy ();
};


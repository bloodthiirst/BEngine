#pragma once
#include "../../Platform/Base/Platform/Platform.h"
#include "../../Application/Application.h"
#include "../Context/RendererContext.h"

class BackendRenderer
{ 
public:
	Application* application;
	Platform* platform;
	size_t frameCount = { 0 };

public:
	virtual bool Startup () = 0;
public:
	virtual void Resize ( uint32_t width, uint32_t height ) = 0;
	virtual bool StartFrame ( RendererContext rendererContext ) = 0;
	virtual bool EndFrame ( RendererContext rendererContext ) = 0;
	
	virtual void Destroy () = 0;
};


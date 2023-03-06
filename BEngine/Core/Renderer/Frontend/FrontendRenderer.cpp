#include "FrontendRenderer.h"
#include "../Backend/BackendRenderer.h"
#include "../../Logger/Logger.h"

FrontendRenderer::FrontendRenderer ( Application* application, Platform* platform, BackendRenderer* backend )
{
	this->application = application;
	this->platform = platform;
	this->backend = backend;
}

bool FrontendRenderer::Startup ()
{
	backend->Startup ();
	return false;
}

bool FrontendRenderer::StartFrame ( RendererContext rendererContext )
{
	return backend->StartFrame(rendererContext);
}

bool FrontendRenderer::EndFrame ( RendererContext rendererContext )
{
	bool result = backend->EndFrame ( rendererContext );
	backend->frameCount++;

	return result;
}

void FrontendRenderer::Resize ( uint32_t width, uint32_t height )
{
	backend->Resize ( width, height );
}

bool FrontendRenderer::DrawFrame ( RendererContext rendererContext )
{
	if (StartFrame ( rendererContext ) )
	{
		if ( !EndFrame ( rendererContext ) )
		{
			Logger::Error ( "Couldn't end frame" );
			return false;
		}
	}

	return true;
}

void FrontendRenderer::Destroy ()
{
	backend->Destroy ();
}

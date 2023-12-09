#include <GameApp.h>
#include "Application.h"
#include "../Global/Global.h"
#include "../Renderer/Backend/BackendRenderer.h"
#include "../Renderer/Backend/Vulkan/Renderer/VulkanBackendRenderer.h"

bool Application::Run()
{
    application_state.isRunning = true;

    while (application_state.isRunning = Global::platform.window.handle_messages())
    {
        // input
        Global::platform.input.OnUpdate( 0 );

        // update game
        game_app.on_update( &game_app, 0 );
        game_app.on_render( &game_app, 0 );

        RendererContext renderer_ctx = RendererContext();

        // draw frame
        {
            if (!Global::backend_renderer.start_frame( &Global::backend_renderer, renderer_ctx ))
            {
                Global::logger.Error( "Couldn't start frame" );
                goto post_frame;
            }

            if (!Global::backend_renderer.end_frame( &Global::backend_renderer, renderer_ctx ))
            {
                Global::logger.Error( "Couldn't end frame" );
                goto post_frame;
            }

            Global::backend_renderer.frame_count++;
            goto post_frame;
        }

    post_frame:
        Global::platform.input.OnPostUpdate( 0 );
    }

    return true;
}
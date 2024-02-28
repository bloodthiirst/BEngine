#include <GameApp.h>
#include "Application.h"
#include "../Defines/Defines.h"
#include "../Global/Global.h"
#include "../Platform/Base/Platform.h"
#include "../Renderer/Backend/BackendRenderer.h"
#include "../Renderer/Backend/Vulkan/Renderer/VulkanBackendRenderer.h"

bool Application::Run()
{
    application_state.isRunning = true;

    Time* time = &Global::platform.time;
    double last_time = Global::platform.time.get_system_time( time );

    while (application_state.isRunning = Global::platform.window.handle_messages())
    {
        double curr_time = Global::platform.time.get_system_time( time );

        float delta = (float) (curr_time - last_time);

        // input
        Global::platform.input.OnUpdate( delta );

        // update game
        game_app.on_update( &game_app, delta );
        
        game_app.on_render( &game_app, delta );

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
        Global::platform.input.OnPostUpdate( delta );
        last_time = curr_time;
    }

    return true;
}
#if _WIN32
#include <windows.h>
#include <synchapi.h>
#endif
#include <GameApp.h>
#include "Application.h"
#include "../Defines/Defines.h"
#include "../Global/Global.h"
#include "../Platform/Base/Platform.h"
#include "../Renderer/Backend/BackendRenderer.h"
#include "../Renderer/VulkanBackend/VulkanBackendRenderer.h"
#include "../AtomicLock/AtomicLock.h"

bool Application::Run()
{
    application_state.is_running = true;

    Time* time = &Global::platform.time;
    double last_time = Global::platform.time.get_system_time( time );

    while ( application_state.is_running = Global::platform.window.handle_messages() )
    {   
        // check file change
        Global::filewatch_ctx.queue_access_lock.Lock();
        {
            Queue<FileWatcher::ChangeInfo>* queue = &Global::filewatch_ctx.file_watcher.changes_queue;
            FileWatcher::ChangeInfo info = {};
            while(Queue<FileWatcher::ChangeInfo>::TryDequeue(queue , &info))
            {
                if(info.change_type == FileWatcher::ChangeFlags::FileLastWriteChanged)
                {
                    printf("File changed detected : %s \n" , info.info.file_modified);
                }
            }
            
        }
        Global::filewatch_ctx.queue_access_lock.Unlock();

        double curr_time = Global::platform.time.get_system_time( time );

        float delta = (float) (curr_time - last_time);

        last_time = curr_time;

        // input
        Global::platform.input.OnUpdate( delta );

        RendererContext renderer_ctx = {};
        DArray<DrawMesh>::Create( 32 ,&renderer_ctx.mesh_draws , Global::alloc_toolbox.frame_allocator);

        // update game
        game_app.on_update( &game_app, delta );

        game_app.on_render( &game_app, &renderer_ctx , delta );


        // draw frame
        {
            if ( !Global::backend_renderer.start_frame( &Global::backend_renderer, &renderer_ctx ) )
            {
                goto post_frame;
            }

            if ( !Global::backend_renderer.draw_frame( &Global::backend_renderer, &renderer_ctx ) )
            {
                Global::logger.Error( "Couldn't draw frame" );
            }

            if ( !Global::backend_renderer.end_frame( &Global::backend_renderer, &renderer_ctx ) )
            {
                Global::logger.Error( "Couldn't end frame" );
                goto post_frame;
            }

            Global::backend_renderer.frame_count++;
            goto post_frame;
        }

    post_frame:
        Global::platform.input.OnPostUpdate( delta );
        Global::alloc_toolbox.frame_arena.offset = 0;
    }

    return true;
}
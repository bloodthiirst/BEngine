#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <profileapi.h>
#endif

#include <cstdint>
#include <Allocators/Allocator.h>
#include <String/StringBuffer.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include <GameApp.h>
#include "Defines/Defines.h"
#include "Global/Global.h"
#include "Application/Application.h"
#include "Logger/Logger.h"
#include "Platform/Base/Platform.h"
#include "Logger/Types/ConsoleLogger.h"
#include "Renderer/Backend/BackendRenderer.h"
#include "Renderer/Backend/Vulkan/Renderer/VulkanBackendRenderer.h"

#ifdef _WIN32
#include "Platform/Types/Win32/Win32Platform.h"
#endif

// Start the game with 500MB of frame arena memory
#define INITIAL_ARENA_CAPACITY 500 * 1'024'000

typedef GameApp( *GenerateGameProc )();

bool TryGetGameDll( ApplicationStartup startup, GameApp* out_game, HMODULE* out_module );

int main( int argc, char** argv )
{
    ApplicationStartup startup = {};

    // get startup params from args 
    {
#ifdef WIN32
        Win32Platform::GetStartupArgs( argv, argc, &startup );
        Win32Platform::Create( &Global::platform );
#endif
    }

    // main allocators
    {
        Global::alloc_toolbox.heap_allocator = HeapAllocator::Create();

        Global::alloc_toolbox.frame_arena = {};
        Global::alloc_toolbox.frame_arena.capacity = INITIAL_ARENA_CAPACITY;
        Global::alloc_toolbox.frame_arena.offset = 0;
        Global::alloc_toolbox.frame_arena.data = Global::alloc_toolbox.heap_allocator.alloc( Global::alloc_toolbox.heap_allocator, INITIAL_ARENA_CAPACITY );
    }

    //Global::platform.startup( &Global::platform );

    // init logger
    {
        Global::logger.Initialize();

        // console logger
        ILogger consoleLogger;
        ConsoleLogger::Create( &consoleLogger );
        Global::logger.Add( consoleLogger );
    }

    // create renderer , only vulkan for now
    {
        VulkanBackendRenderer::Create( &Global::backend_renderer );
    }

    // init core global systems, order matters
    {
        Global::event_system.Startup();
        Global::platform.window.startup_callback(  &Global::platform.window , startup );
        Global::backend_renderer.startup( &Global::backend_renderer , startup );
    }

    GameApp client_game = {};
    HMODULE hmodule = nullptr;
    
    if (!TryGetGameDll( startup, &client_game, &hmodule ))
    {
        Global::logger.Fatal( "Couldn't find Game Dll" );
        goto cleanup;
    }

    Global::logger.Log( "Welcome to BEngine" );

    client_game.initialize( &client_game );

    Global::app.game_app = client_game;
    Global::app.Run();

cleanup:

    if (client_game.destroy)
    {
        client_game.destroy( &client_game );
    }

    client_game = {};

    if (hmodule)
    {
        FreeLibrary( hmodule );
        hmodule = nullptr;
    }

    Global::logger.Destroy();
    Global::backend_renderer.destroy( &Global::backend_renderer );
    Global::event_system.Destroy();

    return 0;
}

bool TryGetGameDll( ApplicationStartup startup, GameApp* out_game, HMODULE* out_module )
{

    GameApp clientGame = {};

    StringBuffer shader_builder_path = StringUtils::Concat( Global::alloc_toolbox.heap_allocator, startup.executable_folder, "\\Assets\\build_shader.bat" );

    StringBuffer sanitized_path = StringUtils::Replace( shader_builder_path.view, "&", "^&", Global::alloc_toolbox.heap_allocator );

    *out_module = LoadLibraryA( "CustomGame.dll" );

    if (out_module == nullptr) {
        *out_game = {};
        *out_module = {};
        return false;
    }

    GenerateGameProc load_game_proc = (GenerateGameProc)GetProcAddress( *out_module, "GetGameApp" );

    if (load_game_proc == nullptr)
    {
        *out_game = {};
        *out_module = {};
        return false;
    }
    *out_game = load_game_proc();
    return true;

}
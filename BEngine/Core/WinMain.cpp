#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#include <profileapi.h>
#endif

#include <cstdint>
#include <Context/CoreContext.h>
#include <Allocators/Allocator.h>
#include <String/StringBuffer.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include <GameApp.h>
#include <Serialization/JSONSerializer.h>
#include <Serialization/XMLSerializer.h>
#include "Defines/Defines.h"
#include "Global/Global.h"
#include "Application/Application.h"
#include "Logger/Logger.h"
#include "Platform/Base/Platform.h"
#include "Logger/Types/ConsoleLogger.h"
#include "Renderer/Backend/BackendRenderer.h"
#include "Renderer/VulkanBackend/VulkanBackendRenderer.h"
#include "AssetManager/GlobalAssetManager.h"
#include "AssetManager/MeshAssetManger.h"
#include "AssetManager/ShaderAssetManager.h"
#include "AssetManager/TextureAssetManager.h"
#include "Command/Command.h"
#include "Renderer/Font/Font.h"
#include "Renderer/RenderGraph/BasicRenderGraph.h"
#include "FileWatcher/FileWatcher.h"
#ifdef _WIN32
#include "Platform/Types/Win32/Win32Platform.h"
#endif

// Start the game with 500MB of frame arena memory
#define INITIAL_GAME_ARENA_CAPACITY 500 * 1'024'000
#define INITIAL_LIB_ARENA_CAPACITY 30 * 1'024'000

typedef GameApp (*GenerateGameProc)();

bool TryGetGameDll(ApplicationStartup startup, GameApp *out_game, HMODULE *out_module);

int main(int argc, char **argv)
{
#ifdef WIN32

    // win32 platform specific init
    {
        Win32Platform::Create(&Global::platform);

        // init the memory func pointers that will be used by the core lib
        {
            CoreContext::free = Global::platform.memory.free;
            CoreContext::malloc = Global::platform.memory.malloc;
            CoreContext::realloc = Global::platform.memory.realloc;
            CoreContext::mem_compare = Global::platform.memory.mem_compare;
            CoreContext::mem_copy = Global::platform.memory.mem_copy;
            CoreContext::mem_init = Global::platform.memory.mem_init;
            CoreContext::mem_set = Global::platform.memory.mem_set;
            CoreContext::mem_move = Global::platform.memory.mem_move;
            CoreContext::core_arena = Arena::Create(INITIAL_LIB_ARENA_CAPACITY);
        }

        // get startup params from args
        Win32Platform::GetStartupArgs(argv, argc, &Global::app.application_startup);
    }
#endif

    assert(!CoreContext::mem_compare(&Global::platform, &Platform{}, sizeof(Platform)));

    // main allocators
    {
        Global::alloc_toolbox.heap_allocator = HeapAllocator::Create();

        Global::alloc_toolbox.frame_arena = Arena::Create(INITIAL_GAME_ARENA_CAPACITY);
        Global::alloc_toolbox.frame_allocator = ArenaAllocator::Create(&Global::alloc_toolbox.frame_arena);
    }

    // Global::platform.startup( &Global::platform );

    // init logger
    {
        Global::logger.Initialize();

        // console logger
        ILogger consoleLogger = {};
        ConsoleLogger::Create(&consoleLogger);
        Global::logger.Add(consoleLogger);
    }

    // create renderer , only vulkan for now
    {
        VulkanBackendRenderer::Create(&Global::backend_renderer);
    }

    // job system
    {
        JobSystem::Create(8 , &Global::job_system);
    }

    // file watcher
    {
        FileWatcher::Params params = {};
        params.directoy_path = Global::app.application_startup.executable_folder;
        params.change_flag_filter = (FileWatcher::ChangeFlags)((size_t)FileWatcher::ChangeFlags::FileNameChanged | (size_t)FileWatcher::ChangeFlags::FileLastWriteChanged);
        params.include_subtree = false;

        FileWatcher::Create(&Global::filewatch_ctx.file_watcher, params);
    }

    // init core global systems, order matters
    {
        Global::event_system.Startup();
        Global::platform.window.startup_callback(&Global::platform.window, Global::app.application_startup);
        Global::asset_manager.Startup();

        // TODO : pass cleanup handle as data maybe
        Thread::ThreadCallback run = [](void *data)
        {
            Global::filewatch_ctx.file_watcher.Start();
            return (DWORD) 0;
        };

        Thread::Create(run, nullptr, &Global::filewatch_ctx.watcher_thread);
        Thread::Run(&Global::filewatch_ctx.watcher_thread);
        
        // mesh asset
        {
            AssetManager manager = {};
            MeshAssetManager::Create(&manager);
            DArray<AssetManager>::Add(&Global::asset_manager.asset_managers, manager);
        }

        // shader asset
        {
            AssetManager manager = {};
            ShaderAssetManager::Create(&manager);
            DArray<AssetManager>::Add(&Global::asset_manager.asset_managers, manager);
        }

        // texture asset
        {
            AssetManager manager = {};
            TextureAssetManager::Create(&manager);
            DArray<AssetManager>::Add(&Global::asset_manager.asset_managers, manager);
        }

        Global::backend_renderer.startup(&Global::backend_renderer, Global::app.application_startup);
    }

    // compile shaders
    {
        StringView shader_folder = "C:\\Dev\\BEngine\\BEngine\\Core\\Resources";

        ArenaCheckpoint check = Global::alloc_toolbox.GetArenaCheckpoint();
        {
            Allocator alloc = Global::alloc_toolbox.frame_allocator;
            DArray<StringBuffer> files{};
            DArray<StringBuffer>::Create(5, &files, alloc);
            Global::platform.filesystem.get_files(shader_folder, &files, alloc);

            StringView glsl_path = "C:\\Dev\\Vulkan\\Bin\\glslc.exe";

            Global::logger.Log("Resource files :");
            for (size_t i = 0; i < files.size; ++i)
            {
                StringView curr = files.data[i].view;

                if (!StringUtils::EndsWith(curr, ".frag") && !StringUtils::EndsWith(curr, ".vert"))
                {
                    continue;
                }

                char temp_buffer[1024] = {0};
                Arena arena = {};
                arena.data = &temp_buffer;
                arena.capacity = 1024;
                arena.offset = 0;
                Allocator temp_alloc = ArenaAllocator::Create(&arena);

                Global::logger.Log(curr);

                StringBuffer full_path = StringUtils::Format(alloc, temp_alloc,"{}\\{}", shader_folder, curr);
                StringBuffer complile_cmd = StringUtils::Format(alloc, temp_alloc, "{} {} -o {}.spv", glsl_path, full_path.view, full_path.view);

                StringBuffer output = {};
                Command cmd = {};
                Command::Create(&cmd, complile_cmd.view);
                Command::Run(cmd, &output, Global::alloc_toolbox.frame_allocator);

                if (output.length != 0)
                {
                    Global::logger.Log(output.view);
                }
            }
        }
        Global::alloc_toolbox.ResetArenaOffset(&check);
    }

    // rendergraph
    {
        VulkanContext *ctx = (VulkanContext *)Global::backend_renderer.user_data;

        RenderGraphBuilder builder = BasicRenderGraph::Create();
        builder.Build(&ctx->render_graph);
    }

    GameApp client_game = {};
    HMODULE hmodule = nullptr;

    if (!TryGetGameDll(Global::app.application_startup, &client_game, &hmodule))
    {
        Global::logger.Fatal("Couldn't find Game Dll");
        goto cleanup;
    }

    Global::logger.Log("Welcome to BEngine");

    client_game.initialize(&client_game);

    Global::app.game_app = client_game;
    Global::app.Run();

cleanup:

    if (client_game.destroy)
    {
        client_game.destroy(&client_game);
    }

    client_game = {};

    if (hmodule)
    {
        FreeLibrary(hmodule);
        hmodule = nullptr;
    }

    JobSystem::Destroy(&Global::job_system);
    FileWatcher::Destroy(&Global::filewatch_ctx.file_watcher);
    Global::backend_renderer.destroy(&Global::backend_renderer);
    Global::asset_manager.Destroy();
    Global::event_system.Destroy();
    Global::logger.Destroy();
    Global::platform.window.destroy();

    return 0;
}

bool TryGetGameDll(ApplicationStartup startup, GameApp *out_game, HMODULE *out_module)
{
    GameApp clientGame = {};

    StringBuffer shader_builder_path = StringUtils::Concat(Global::alloc_toolbox.heap_allocator, startup.executable_folder, "\\Assets\\build_shader.bat");

    StringBuffer sanitized_path = StringUtils::Replace(shader_builder_path.view, "&", "^&", Global::alloc_toolbox.heap_allocator);

    *out_module = LoadLibraryA("CustomGame.dll");

    if (out_module == nullptr)
    {
        *out_game = {};
        *out_module = {};
        return false;
    }

    GenerateGameProc load_game_proc = (GenerateGameProc)GetProcAddress(*out_module, "GetGameApp");

    if (load_game_proc == nullptr)
    {
        *out_game = {};
        *out_module = {};
        return false;
    }

    *out_game = load_game_proc();
    return true;
}
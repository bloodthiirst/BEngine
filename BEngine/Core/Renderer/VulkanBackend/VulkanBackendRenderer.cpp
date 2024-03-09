#include "VulkanBackendRenderer.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <Maths/Maths.h>
#include <Maths/Rect.h>
#include <Maths/Color.h>
#include <Maths/Vector3.h>
#include <String/StringView.h>
#include <String/StringUtils.h>
#include "../../Defines/Defines.h"
#include "../../Global/Global.h"
#include "../../Logger/Logger.h"
#include "../../Platform/Base/Platform.h"
#include "../../Platform/Base/Window.h"
#include "../../Platform/Types/Win32/Win32Platform.h"
#include "../Backend/BackendRenderer.h"
#include "../Context/RendererContext.h"
#include "../Context/VulkanContext.h"
#include "../CommandBuffer/CommandBuffer.h"
#include "../DescriptorManager/DescriptorManager.h"
#include "../Fence/Fence.h"
#include "../Texture/Texture.h"
#include "../Shader/Shader.h"
#include "../Buffer/Buffer.h"
#include "../Renderpasses/BasicRenderpass.h"
#include <Maths/Vector2.h>
#include "../Mesh/Vertex3D.h"

struct PhysicalDeviceRequirements
{
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    bool samplerAnisotropy;
    bool discreteGPU;
};

struct LogicalDeviceRequirements
{
    bool samplerAnisotropy;
};

const char* INSATNCE_EXTENSION[] =
{
    VK_KHR_SURFACE_EXTENSION_NAME,

#if defined(_WIN32)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

#if defined(_DEBUG)
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};
const char* VK_LAYERS[] = { "VK_LAYER_KHRONOS_validation" };

const char* DEVICE_EXTENSIONS[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME ,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME
};

VKAPI_ATTR VkBool32 DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData )
{
    switch ( severity )
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        Global::logger.Error( callbackData->pMessage );
        break;
    }

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    {
        Global::logger.Log( callbackData->pMessage );
        break;
    }

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        Global::logger.Info( callbackData->pMessage );
        break;
    }

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        Global::logger.Warning( callbackData->pMessage );
        break;
    }
    }

    return VK_FALSE;
}


bool HasExtensions( VkExtensionProperties* extensionsArr, uint32_t extensionCount, const char** requiredExt, uint32_t requiredCount )
{
    for ( uint32_t i = 0; i < requiredCount; i++ )
    {
        auto req = requiredExt[i];

        bool foundExt = false;

        for ( uint32_t j = 0; j < extensionCount; j++ )
        {
            auto ext = extensionsArr[j];
            if ( strcmp( ext.extensionName, req ) == 0 )
            {
                foundExt = true;
                break;
            }
        }

        if ( !foundExt )
            return false;
    }

    return true;
}

bool CreateSurface( VulkanContext* context, VkSurfaceKHR* surface )
{
#ifdef _WIN32

    Win32WindowState* win32plat = (Win32WindowState*) Global::platform.window.user_data;

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = win32plat->process_handle;
    createInfo.hwnd = win32plat->window_handle;

    VK_CHECK( vkCreateWin32SurfaceKHR( context->vulkanInstance, &createInfo, nullptr, surface ), result );

    return result == VK_SUCCESS;

#else
    return false;
#endif
}

bool CreateLogicalDevice( VkInstance vkInstance, VkSurfaceKHR surface, PhysicalDeviceInfo* physicalDeviceinfo, LogicalDeviceRequirements* requirements, const VkAllocationCallbacks* allocator, VkDevice* handle )
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( physicalDeviceinfo->handle, &queueFamilyCount, nullptr );

    Allocator alloc = STACK_ALLOC_ARRAY( int, queueFamilyCount );
    int* queuesNeededPerFamily = (int*) alloc.alloc( alloc, queueFamilyCount * sizeof( int ) );

    // for now , if multiple requirement share a queue index , only one queue will used for them
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.computeQueueFamilyIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.graphicsQueueIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.presentQueueFamilyIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.transferQueueIndex]++;

    Allocator alloc_create = STACK_ALLOC_ARRAY( VkDeviceQueueCreateInfo, queueFamilyCount );
    DArray<VkDeviceQueueCreateInfo> queueCreationInfos;
    DArray<VkDeviceQueueCreateInfo>::Create( queueFamilyCount, &queueCreationInfos, alloc_create );

    for ( uint32_t i = 0; i < queueFamilyCount; ++i )
    {
        // no queues needed from this family
        if ( queuesNeededPerFamily[i] == 0 )
            continue;


        VkDeviceQueueCreateInfo createQueueInfo = {};
        createQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        createQueueInfo.queueFamilyIndex = i;
        createQueueInfo.queueCount = 1;

        /*
        if ( i == physicalDeviceinfo->queueFamilyInfo.graphicsQueueIndex )
        {
            createQueueInfo.queueCount = 2;
        }
        */
        float* queuePrio = Global::alloc_toolbox.HeapAlloc<float>( createQueueInfo.queueCount, true );

        createQueueInfo.pQueuePriorities = queuePrio;
        DArray< VkDeviceQueueCreateInfo>::Add( &queueCreationInfos, createQueueInfo );
    }

    // todo : we can fill this struct to request more device features
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = requirements->samplerAnisotropy;

    VkDeviceCreateInfo createDeviceInfo = {};
    createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createDeviceInfo.queueCreateInfoCount = (uint32_t) queueCreationInfos.size;
    createDeviceInfo.pQueueCreateInfos = queueCreationInfos.data;

    createDeviceInfo.pEnabledFeatures = &deviceFeatures;
    createDeviceInfo.enabledExtensionCount = sizeof( DEVICE_EXTENSIONS ) / sizeof( char* );
    createDeviceInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;

    // these 2 features are deprecated/ignored
    createDeviceInfo.enabledLayerCount = 0;
    createDeviceInfo.ppEnabledLayerNames = nullptr;

    VK_CHECK( vkCreateDevice( physicalDeviceinfo->handle, &createDeviceInfo, allocator, handle ), result );

    // free the prio queues
    for ( uint32_t i = 0; i < queueCreationInfos.size; ++i )
    {
        Global::alloc_toolbox.HeapFree( queueCreationInfos.data[i].pQueuePriorities );
    }

    if ( result != VK_SUCCESS )
    {
        return false;
    }

    vkGetDeviceQueue( *handle, physicalDeviceinfo->queuesInfo.graphicsQueueIndex, 0, &physicalDeviceinfo->queuesInfo.graphicsQueue );
    vkGetDeviceQueue( *handle, physicalDeviceinfo->queuesInfo.computeQueueFamilyIndex, 0, &physicalDeviceinfo->queuesInfo.computeQueue );
    vkGetDeviceQueue( *handle, physicalDeviceinfo->queuesInfo.presentQueueFamilyIndex, 0, &physicalDeviceinfo->queuesInfo.presentQueue );
    vkGetDeviceQueue( *handle, physicalDeviceinfo->queuesInfo.transferQueueIndex, 0, &physicalDeviceinfo->queuesInfo.transferQueue );

    return true;
}

bool PhysicalDeviceHasSwapchainSupport( VkPhysicalDevice handle, VkSurfaceKHR surface, SwapchainSupportInfo* outSwapchainInfo )
{
    Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

    // capabilities
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( handle, surface, &capabilities );

    // surface formats
    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( handle, surface, &formatsCount, nullptr );
    DArray<VkSurfaceFormatKHR> formats;
    DArray<VkSurfaceFormatKHR>::Create( formatsCount, &formats, heap_alloc );
    vkGetPhysicalDeviceSurfaceFormatsKHR( handle, surface, &formatsCount, formats.data );

    // present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR( handle, surface, &presentModeCount, nullptr );
    DArray<VkPresentModeKHR> presentModes;
    DArray<VkPresentModeKHR>::Create( presentModeCount, &presentModes, heap_alloc );

    vkGetPhysicalDeviceSurfacePresentModesKHR( handle, surface, &presentModeCount, presentModes.data );

    // todo : here we add a check for device extensions

    outSwapchainInfo->capabilities = capabilities;
    outSwapchainInfo->presentModes = presentModes;
    outSwapchainInfo->surfaceFormats = formats;

    return true;
}

bool CreatePhysicalDevice( Platform* platform, VkInstance vkInstance, VkSurfaceKHR* surface, PhysicalDeviceInfo* outDeviceInfo )
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices( vkInstance, &physicalDeviceCount, nullptr );

    Allocator alloc_info = STACK_ALLOC_ARRAY( PhysicalDeviceRequirements, physicalDeviceCount );
    PhysicalDeviceRequirements* infos = (PhysicalDeviceRequirements*) alloc_info.alloc( alloc_info, physicalDeviceCount * sizeof( PhysicalDeviceRequirements ) );

    Allocator alloc_devices = STACK_ALLOC_ARRAY( VkPhysicalDevice, physicalDeviceCount );
    VkPhysicalDevice* physicalDevices = (VkPhysicalDevice*) alloc_devices.alloc( alloc_devices, physicalDeviceCount * sizeof( VkPhysicalDevice ) );

    vkEnumeratePhysicalDevices( vkInstance, &physicalDeviceCount, physicalDevices );

    PhysicalDeviceRequirements requirements = {};
    requirements.graphics = true;
    requirements.present = true;
    requirements.compute = true;
    requirements.transfer = true;
    requirements.samplerAnisotropy = true;
    requirements.discreteGPU = true;

    int maxScore = 0;

    Global::logger.Log( "Selecting physical device ...." );

    for ( uint32_t i = 0; i < physicalDeviceCount; ++i )
    {
        VkPhysicalDevice currPhysicalDevice = physicalDevices[i];
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memory;

        vkGetPhysicalDeviceProperties( currPhysicalDevice, &props );
        vkGetPhysicalDeviceFeatures( currPhysicalDevice, &features );
        vkGetPhysicalDeviceMemoryProperties( currPhysicalDevice, &memory );

        Global::logger.Log( "> Evaluating device : {}", props.deviceName );

        // discrete
        if ( (requirements.discreteGPU) && (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) )
        {
            Global::logger.Error( "This device is not a discrete GPU , skippping ..." );
            continue;
        }

        if ( (requirements.samplerAnisotropy) && (features.samplerAnisotropy == VK_FALSE) )
        {
            Global::logger.Error( "This device does not support sampler anisotropy , skippping ..." );
            continue;
        }

        // queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( currPhysicalDevice, &queueFamilyCount, nullptr );

        Allocator alloc_queue = STACK_ALLOC_ARRAY( VkQueueFamilyProperties, queueFamilyCount );
        VkQueueFamilyProperties* queueProps = (VkQueueFamilyProperties*) alloc_queue.alloc( alloc_queue, queueFamilyCount * sizeof( VkQueueFamilyProperties ) );

        vkGetPhysicalDeviceQueueFamilyProperties( currPhysicalDevice, &queueFamilyCount, queueProps );

        size_t mem_size = queueFamilyCount * sizeof( float );

        Allocator alloc_score = EmplaceAllocator::Create( _alloca( mem_size ) );
        Allocator alloc_graph = STACK_ALLOC( mem_size );
        Allocator alloc_transfer = STACK_ALLOC( mem_size );
        Allocator alloc_compute = STACK_ALLOC( mem_size );

        float* presentScore = (float*) alloc_score.alloc( alloc_score, mem_size );
        float* graphicsScore = (float*) alloc_graph.alloc( alloc_graph, mem_size );
        float* transferScore = (float*) alloc_transfer.alloc( alloc_transfer, mem_size );
        float* computeScore = (float*) alloc_compute.alloc( alloc_compute, mem_size );

        Global::platform.memory.mem_init( presentScore, mem_size );
        Global::platform.memory.mem_init( graphicsScore, mem_size );
        Global::platform.memory.mem_init( transferScore, mem_size );
        Global::platform.memory.mem_init( computeScore, mem_size );

        // the idea here is to use the "least" diverse queues first
        // so we assign the queue families a score based on their : count , flags
        for ( uint32_t i = 0; i < queueFamilyCount; ++i )
        {
            VkQueueFamilyProperties currQueue = queueProps[i];

            float currScore = 0;
            // the less queue are available , the higher the score

            // compute
            if ( currQueue.queueFlags & VK_QUEUE_COMPUTE_BIT )
            {
                currScore = (float) 1 / currQueue.queueCount;
            }
            computeScore[i] = currScore;

            // gprahics
            currScore = 0;
            if ( currQueue.queueFlags & VK_QUEUE_GRAPHICS_BIT )
            {
                // the less queue are available , the higher the score
                currScore = (float) 1 / currQueue.queueCount;
            }
            graphicsScore[i] = currScore;

            // transfer
            currScore = 0;
            if ( currQueue.queueFlags & VK_QUEUE_TRANSFER_BIT )
            {
                // the less queue are available , the higher the score
                currScore = (float) 1 / currQueue.queueCount;
            }
            transferScore[i] = currScore;

            // present
            VkBool32 presentSupported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR( currPhysicalDevice, i, *surface, &presentSupported );

            currScore = 0;
            if ( presentSupported == VK_TRUE )
            {
                currScore = (float) 1 / currQueue.queueCount;
            }

            presentScore[i] = currScore;
        }

        Allocator alloc_empty = STACK_ALLOC( mem_size );
        float* emptyScore = (float*) alloc_empty.alloc( alloc_empty, mem_size );
        Global::platform.memory.mem_init( emptyScore, mem_size );


        if ( (requirements.present) && Global::platform.memory.mem_compare( presentScore, emptyScore, mem_size ) )
        {
            Global::logger.Error( "This device does't support present , skipping ..." );
            continue;
        }

        if ( (requirements.graphics) && Global::platform.memory.mem_compare( graphicsScore, emptyScore, mem_size ) )
        {
            Global::logger.Error( "This device does't support graphics , skipping ..." );
            continue;
        }
        if ( (requirements.transfer) && Global::platform.memory.mem_compare( transferScore, emptyScore, mem_size ) )
        {
            Global::logger.Error( "This device does't support transfer , skipping ..." );
            continue;
        }
        if ( (requirements.compute) && Global::platform.memory.mem_compare( computeScore, emptyScore, mem_size ) )
        {
            Global::logger.Error( "This device does't support compute , skipping ..." );
            continue;
        }

        // here we found a GPU that supports all of our requirements
        // so we select the optimal queue index for each requirement
        int presentQueueFamilyIndex = 0;
        int graphicsQueueFamilyIndex = 0;
        int computeQueueFamilyIndex = 0;
        int transferQueueFamilyIndex = 0;

        for ( uint32_t i = 1; i < queueFamilyCount; ++i )
        {
            if ( presentScore[i] > presentScore[presentQueueFamilyIndex] )
            {
                presentQueueFamilyIndex = i;
            }


            if ( graphicsScore[i] > graphicsScore[graphicsQueueFamilyIndex] )
            {
                graphicsQueueFamilyIndex = i;
            }


            if ( computeScore[i] > computeScore[computeQueueFamilyIndex] )
            {
                computeQueueFamilyIndex = i;
            }


            if ( transferScore[i] > transferScore[transferQueueFamilyIndex] )
            {
                transferQueueFamilyIndex = i;
            }
        }

        SwapchainSupportInfo swapchainSupportInfo = {};

        // now we check for swapchain support
        if ( !PhysicalDeviceHasSwapchainSupport( currPhysicalDevice, *surface, &swapchainSupportInfo ) )
        {
            Global::logger.Error( "This device doesn't support swapchain , skipping ..." );
            continue;
        }

        // if we get here , the physical device is usable
        outDeviceInfo->handle = currPhysicalDevice;
        outDeviceInfo->physicalDeviceFeatures = features;
        outDeviceInfo->physicalDeviceMemoryProperties = memory;
        outDeviceInfo->physicalDeviceProperties = props;
        outDeviceInfo->swapchainSupportInfo = swapchainSupportInfo;
        outDeviceInfo->queuesInfo.presentQueueFamilyIndex = computeQueueFamilyIndex;
        outDeviceInfo->queuesInfo.computeQueueFamilyIndex = computeQueueFamilyIndex;
        outDeviceInfo->queuesInfo.graphicsQueueIndex = graphicsQueueFamilyIndex;
        outDeviceInfo->queuesInfo.transferQueueIndex = transferQueueFamilyIndex;

        return true;
    }

    // if we get here , no physical device satisfies the requirements

    return false;
}

bool CreateGraphicsCommandPools( VulkanContext* context )
{
    VkCommandPoolCreateInfo graphicsPoolCreate = {};
    graphicsPoolCreate.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsPoolCreate.queueFamilyIndex = context->physicalDeviceInfo.queuesInfo.graphicsQueueIndex;
    graphicsPoolCreate.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;


    vkCreateCommandPool( context->logicalDeviceInfo.handle, &graphicsPoolCreate, context->allocator, &context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool );
    return true;
}

bool UploadDataRange( VulkanContext* context, VkCommandPool pool, Fence fence, VkQueue queue, Buffer* inBuffer, uint32_t offset, uint32_t size, void* inDataPtr )
{
    // first , we create a host-visible staging buffer to upload the data to in
    // then we mark it as the source of the transfer
    VkMemoryPropertyFlagBits usage = (VkMemoryPropertyFlagBits) (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Buffer stagingBuffer = {};

    BufferDescriptor bufferDesc = {};
    bufferDesc.size = size;
    bufferDesc.memoryPropertyFlags = usage;
    bufferDesc.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Buffer::Create( context, bufferDesc, true, &stagingBuffer );

    Buffer::Load( context, 0, size, inDataPtr, 0, &stagingBuffer );

    Buffer::Copy( context, pool, fence, queue, &stagingBuffer, 0, inBuffer, offset, size );

    Buffer::Destroy( context, &stagingBuffer );

    return true;
}


void HandleWindowResize( WindowResizeEvent  evt )
{
    Global::backend_renderer.resize( &Global::backend_renderer, evt.dimensions.x, evt.dimensions.y );
}

bool Startup( BackendRenderer* in_renderer, ApplicationStartup startup )
{
    VulkanContext* ctx = Global::alloc_toolbox.HeapAlloc<VulkanContext>();
    in_renderer->user_data = ctx;

    ctx->allocator = nullptr;
    ctx->frameBufferSize.x = Global::platform.window.width;
    ctx->frameBufferSize.y = Global::platform.window.height;

    Global::event_system.Listen<WindowResizeEvent>( HandleWindowResize );

    // check for extensions
    uint32_t instance_extCount = 0;
    vkEnumerateInstanceExtensionProperties( nullptr, &instance_extCount, nullptr );

    VkExtensionProperties* instance_exts = (VkExtensionProperties*) _alloca( sizeof( VkExtensionProperties ) * instance_extCount );
    vkEnumerateInstanceExtensionProperties( nullptr, &instance_extCount, instance_exts );

    Global::logger.Log( "Instance extensions available : {} ", instance_extCount );

    for ( uint32_t i = 0; i < instance_extCount; ++i )
    {
        Global::logger.Log( instance_exts[i].extensionName );
    }

    // info about our app
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Defines::engine_name;
    appInfo.pEngineName = Defines::engine_name;

    appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_0;


    // creation info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    // layers
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = VK_LAYERS;

    // extensions
    createInfo.enabledExtensionCount = sizeof( INSATNCE_EXTENSION ) / sizeof( char* );
    createInfo.ppEnabledExtensionNames = INSATNCE_EXTENSION;

    VkResult result = vkCreateInstance( &createInfo, nullptr, &ctx->vulkanInstance );

    if ( result != VK_SUCCESS )
    {
        return false;
    }
#if defined(_DEBUG)
    int loggingSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;

    int messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugInfoCreate;
    debugInfoCreate.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugInfoCreate.messageSeverity = loggingSeverity;
    debugInfoCreate.messageType = messageType;
    debugInfoCreate.pUserData = nullptr;
    debugInfoCreate.flags = 0;
    debugInfoCreate.pNext = nullptr;
    debugInfoCreate.pfnUserCallback = DebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr( ctx->vulkanInstance, "vkCreateDebugUtilsMessengerEXT" );
    func( ctx->vulkanInstance, &debugInfoCreate, ctx->allocator, &ctx->debugMessenger );

    Global::logger.NewLine( 1 );

    Global::logger.Log( "Vulkan Debugger created" );

#endif
    Global::logger.NewLine( 1 );


    Global::logger.Log( "Logical extensions needed :" );
    for ( const auto& ext : DEVICE_EXTENSIONS )
    {
        Global::logger.Log( ext );
    }

    // surface
    if ( !CreateSurface( ctx, &ctx->surface ) )
    {
        Global::logger.Error( "Couldn't create Vulkan Surface ..." );
        return false;
    }

    Global::logger.Log( "Vulkan surface created" );
    Global::logger.NewLine();

    PhysicalDeviceInfo physicalDeviceInfo = {};

    if ( !CreatePhysicalDevice( &Global::platform, ctx->vulkanInstance, &ctx->surface, &physicalDeviceInfo ) )
    {
        Global::logger.Error( "Couldn't create Physical device ..." );
        return false;
    }

    ctx->physicalDeviceInfo = physicalDeviceInfo;

    Global::logger.NewLine();

    Global::logger.Info( "> Selected physical device" );
    Global::logger.Info( "Name : {}", physicalDeviceInfo.physicalDeviceProperties.deviceName );
    Global::logger.Info( "Max uniform Buffer range : {} ", physicalDeviceInfo.physicalDeviceProperties.limits.maxUniformBufferRange );

    const char* deviceType = {};
    switch ( physicalDeviceInfo.physicalDeviceProperties.deviceType )
    {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
    {
        deviceType = "Discrete";
        break;
    }
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
    {
        deviceType = "CPU";
        break;
    }
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
    {
        deviceType = "Intergrated";
        break;
    }
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
    {
        deviceType = "Other";
        break;
    }
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
    {
        deviceType = "Virtual GPU";
        break;
    }
    }

    Global::logger.Info( "GPU Type : {} ", deviceType );

    Global::logger.Info( "GPU Driver version : {}.{}.{}",
                         (uint32_t) VK_API_VERSION_MAJOR( physicalDeviceInfo.physicalDeviceProperties.driverVersion ),
                         (uint32_t) VK_API_VERSION_MINOR( physicalDeviceInfo.physicalDeviceProperties.driverVersion ),
                         (uint32_t) VK_API_VERSION_PATCH( physicalDeviceInfo.physicalDeviceProperties.driverVersion )
    );


    Global::logger.Info( "Vulkan API version : {}.{}.{}",
                         VK_API_VERSION_MAJOR( physicalDeviceInfo.physicalDeviceProperties.apiVersion ),
                         VK_API_VERSION_MINOR( physicalDeviceInfo.physicalDeviceProperties.apiVersion ),
                         VK_API_VERSION_PATCH( physicalDeviceInfo.physicalDeviceProperties.apiVersion )
    );

    Global::logger.NewLine();

    LogicalDeviceRequirements logicalRequirements = {};
    logicalRequirements.samplerAnisotropy = true;

    VkDevice handle = {};
    if ( !CreateLogicalDevice( ctx->vulkanInstance, ctx->surface, &ctx->physicalDeviceInfo, &logicalRequirements, ctx->allocator, &handle ) )
    {
        Global::logger.Error( "Couldn't create logical device ...." );
        return false;
    }

    ctx->logicalDeviceInfo.handle = handle;

    Global::logger.Info( "Logical device and queue created" );

    // create graphics command pool
    if ( !CreateGraphicsCommandPools( ctx ) )
    {
        Global::logger.Error( "Couldn't create command pools ...." );
        return false;
    }

    Global::logger.Info( "Command pools created" );

    // create swapshain
    SwapchainCreateDescription swapchainDesc = {};
    swapchainDesc.width = Global::platform.window.width;
    swapchainDesc.height = Global::platform.window.height;
    swapchainDesc.imagesCount = 3;

    if ( !SwapchainInfo::Create( ctx, swapchainDesc, VK_NULL_HANDLE, &ctx->swapchain_info ) )
    {
        Global::logger.Error( "Couldn't create swapchain ...." );
        return false;
    }

    Global::logger.Info( "Swapchain created" );

    // create renderpass

    Rect rect = {};
    rect.x = 0;
    rect.y = 0;
    rect.width = (float) Global::platform.window.width;
    rect.height = (float) Global::platform.window.height;

    Color clearColor = {};
    clearColor.r = 0;
    clearColor.g = 0;
    clearColor.b = 0.2f;
    clearColor.a = 1;

    BasicRenderpassParams renderpass_params = {};
    renderpass_params.area = rect;
    renderpass_params.clearColor = clearColor;
    renderpass_params.depth = 1;
    renderpass_params.stencil = 0;
    renderpass_params.sync_window_size = true;

    if ( !BasicRenderpass::Create( ctx, renderpass_params, &ctx->renderPass ) )
    {
        Global::logger.Error( "Couldn't create renderpass ...." );
        return false;
    }

    Global::logger.Info( "Renderpass created" );

    // create descriptor set pools
    {
        DescriptorManager::Create( ctx, &ctx->descriptor_manager );
        ctx->descriptor_manager.init_sets_count = 100;
        ctx->descriptor_manager.resize_factor = 2;
    }

    // build shader
    {
        Allocator alloc = HeapAllocator::Create();

        StringView vert_path = "C:\\Dev\\BEngine\\BEngine\\Core\\Resources\\SimpleShader.vert.spv";
        StringView frag_path = "C:\\Dev\\BEngine\\BEngine\\Core\\Resources\\SimpleShader.frag.spv";

        FileHandle vert_handle = {};
        FileHandle frag_handle = {};

        if ( !Global::platform.filesystem.open( vert_path, FileMode::Read, true, &vert_handle ) )
        {
            Global::logger.Error( "Can't find vertex shader" );
            return false;
        }

        if ( !Global::platform.filesystem.open( frag_path, FileMode::Read, true, &frag_handle ) )
        {
            Global::logger.Error( "Can't find frag shader" );
            return false;
        }

        size_t vert_size = {};
        size_t frag_size = {};

        Global::platform.filesystem.get_size( &vert_handle, &vert_size );
        Global::platform.filesystem.get_size( &frag_handle, &frag_size );

        StringBuffer vert_code = StringBuffer::Create( vert_size, alloc );
        StringBuffer frag_code = StringBuffer::Create( frag_size, alloc );

        size_t bytes_read = {};
        Global::platform.filesystem.read_all( vert_handle, vert_code.buffer, &bytes_read );
        Global::platform.filesystem.read_all( frag_handle, frag_code.buffer, &bytes_read );

        bool created = ShaderBuilder::Create()
            .SetStage( VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, vert_code.view )
            .SetStage( VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT, frag_code.view )
            .AddVertexAttribute( "position", 0, sizeof( Vector3 ), VkFormat::VK_FORMAT_R32G32B32_SFLOAT )
            .AddVertexAttribute( "texcoord", 1, sizeof( Vector2 ), VkFormat::VK_FORMAT_R32G32_SFLOAT )
            .AddDescriptor( "global_ubo", 0, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT )
            //.AddDescriptor( "object_ubo", 1, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
            .AddDescriptor( "diffuse_sampler", 1, 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT )
            .Build( ctx, &ctx->default_shader );

        StringBuffer::Destroy( &vert_code );
        StringBuffer::Destroy( &frag_code );

        if ( vert_handle.is_valid )
        {
            Global::platform.filesystem.close( &vert_handle );
        }

        if ( frag_handle.is_valid )
        {
            Global::platform.filesystem.close( &frag_handle );
        }

        if ( !created )
        {
            return false;
        }
    }

    Global::logger.Info( "Default shader created" );

    // create sampler
    {
        VkSamplerCreateInfo create_sampler = { };
        create_sampler.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        create_sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_sampler.unnormalizedCoordinates = VK_FALSE;
        create_sampler.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_sampler.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_sampler.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_sampler.magFilter = VkFilter::VK_FILTER_LINEAR;
        create_sampler.anisotropyEnable = VK_FALSE;
        create_sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        vkCreateSampler( ctx->logicalDeviceInfo.handle, &create_sampler, ctx->allocator, &ctx->default_sampler );
    }

    Global::logger.Info( "Default sampler created" );

    // create texture
    {
        size_t width = 64;
        size_t height = 64;
        size_t cell_size = 4;

        TextureDescriptor tex_desc = {};
        tex_desc.create_view = true;
        tex_desc.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
        tex_desc.height = (uint32_t) height;
        tex_desc.width = (uint32_t) width;
        tex_desc.view_aspect_flags = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
        tex_desc.image_type = VkImageType::VK_IMAGE_TYPE_2D;
        tex_desc.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
        tex_desc.usage = (VkImageUsageFlagBits) (VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);

        Texture::Create( ctx, tex_desc, &ctx->default_texture );

        Color* colors = Global::alloc_toolbox.HeapAlloc<Color>( width * height );

        Color dark = { 1,1,1,1 };
        Color light = { 0,0,0,1 };

        for ( size_t y = 0; y < width; ++y )
        {
            bool is_y_odd = (y / cell_size) % 2;

            for ( size_t x = 0; x < width; ++x )
            {
                bool is_x_odd = (x / cell_size) % 2;

                bool col = is_x_odd ^ is_y_odd;

                colors[x + (y * width)] = col ? dark : light;
            }
        }

        BufferDescriptor desc = {};
        desc.size = (uint32_t) (width * height * sizeof( Color ));
        desc.memoryPropertyFlags = (VkMemoryPropertyFlagBits) (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        desc.usage = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

        Buffer copy_buffer = {};
        Buffer::Create( ctx, desc, true, &copy_buffer );
        Buffer::Load( ctx, 0, (uint32_t) (width * height * sizeof( Color )), colors, 0, &copy_buffer );


        CommandBuffer cmd = {};
        VkCommandPool pool = ctx->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool;
        CommandBuffer::SingleUseAllocateBegin( ctx, pool, &cmd );
        Texture::TransitionLayout( ctx, &ctx->default_texture, cmd, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
        Texture::CopyFromBuffer( ctx, copy_buffer.handle, &ctx->default_texture, cmd );
        Texture::TransitionLayout( ctx, &ctx->default_texture, cmd, VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
        CommandBuffer::SingleUseEndSubmit( ctx, pool, &cmd, ctx->physicalDeviceInfo.queuesInfo.graphicsQueue );
    }

    Global::logger.Info( "Default texture created" );

    // vertex buffer
    {
        BufferDescriptor bufferDesc = {};

        VkBufferUsageFlagBits usage = (VkBufferUsageFlagBits)
            (
                VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                );

        bufferDesc.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bufferDesc.size = sizeof( Vertex3D ) * 1024 * 1024;
        bufferDesc.usage = usage;

        if ( !Buffer::Create( ctx, bufferDesc, true, &ctx->vertexBuffer ) )
        {
            Global::logger.Error( "Couldn't create vertex buffer ...." );
            return false;
        }

        ctx->geometryVertexOffset = 0;
        Global::logger.Info( "Vertex buffer created" );
    }

    // index buffer
    {
        BufferDescriptor bufferDesc = {};

        VkBufferUsageFlagBits usage = (VkBufferUsageFlagBits)
            (
                VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                );

        bufferDesc.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        bufferDesc.size = sizeof( uint32_t ) * 1024 * 1024;
        bufferDesc.usage = usage;

        if ( !Buffer::Create( ctx, bufferDesc, true, &ctx->indexBuffer ) )
        {
            Global::logger.Error( "Couldn't create index buffer ...." );
            return false;
        }

        ctx->indexBufferSize = bufferDesc.size;
        Global::logger.Info( "Index buffer created" );
    }

    // test mesh
    {
        const uint32_t vert_count = 4;
        const uint32_t index_count = 6;

        Vertex3D vertPositions[vert_count] = {
            { Vector3( +0.5f , +0.5f , 0.0 ), Vector2( 1.0f , 1.0f )},
            { Vector3( -0.5f , +0.5f , 0.0 ), Vector2( 0.0f , 1.0f )},
            { Vector3( +0.5f , -0.5f , 0.0 ), Vector2( 1.0f , 0.0f )},
            { Vector3( -0.5f , -0.5f , 0.0 ), Vector2( 0.0f , 0.0f )}
        };

        uint32_t vertIndicies[6] = {
            0 ,
            1 ,
            2 ,

            3 ,
            1 ,
            2 ,
        };

        uint32_t vertex_size = sizeof( Vertex3D ) * vert_count;
        uint32_t indicies_size = sizeof( uint32_t ) * index_count;

        UploadDataRange( ctx, ctx->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, {}, ctx->physicalDeviceInfo.queuesInfo.graphicsQueue, &ctx->vertexBuffer, 0, vertex_size, vertPositions );
        UploadDataRange( ctx, ctx->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, {}, ctx->physicalDeviceInfo.queuesInfo.graphicsQueue, &ctx->indexBuffer, 0, indicies_size, vertIndicies );
    }

    return true;
}


void Resize( BackendRenderer* in_backend, uint32_t width, uint32_t height )
{
    VulkanContext* ctx = (VulkanContext*) in_backend->user_data;
    ctx->frameBufferSize.x = width;
    ctx->frameBufferSize.y = height;
    ctx->frameBufferSizeCurrentGeneration++;
}

// Recreate the swapchain if needed
// Waits for the last frame's submittion if needed
// Acquires and updates the new image index from the swapchain
// current frame is still the same , it gets updated after "Present"
bool StartFrame( BackendRenderer* in_backend, RendererContext rendererContext )
{
    VulkanContext* ctx = (VulkanContext*) in_backend->user_data;
    LogicalDeviceInfo device = ctx->logicalDeviceInfo;

    if ( ctx->recreateSwapchain )
    {
        VkResult result = vkDeviceWaitIdle( device.handle );

        if ( result != VkResult::VK_SUCCESS )
        {
            Global::logger.Error( "Couldn't wait device idle" );
        }

        return false;
    }

    // check if we need to recreate the swapchain
    if ( ctx->frameBufferSizeCurrentGeneration != ctx->frameBufferSizeLastGeneration )
    {
        VkResult result = vkDeviceWaitIdle( device.handle );

        if ( result != VkResult::VK_SUCCESS )
        {
            Global::logger.Error( "Couldn't wait device idle" );
            return false;
        }

        SwapchainCreateDescription desc = {};
        desc.width = ctx->frameBufferSize.x;
        desc.height = ctx->frameBufferSize.y;
        desc.imagesCount = 3;
        SwapchainInfo::Recreate( ctx, desc, &ctx->swapchain_info );

        return false;
    }

    // first we get the index of the last frame index
    uint32_t last_frame = ctx->current_frame;

    // we wait until the previous frame is done with it's command buffers (or presentation)
    if ( !ctx->swapchain_info.cmd_buffer_done_execution_per_frame.data[last_frame].Wait( ctx, UINT64_MAX ) )
    {
        return false;
    }

    // we ask the swapchain to get us the index of an image that we can render to  
    // plug last frame's presentaion semaphore as a "dependency" (in other words , make sure last presentation is donee)
    uint32_t current_image = {};
    if ( !ctx->swapchain_info.AcquireNextImageIndex( ctx, UINT32_MAX, ctx->swapchain_info.image_presentation_complete_semaphores.data[last_frame], nullptr, &current_image ) )
    {
        Global::logger.Error( "Couldn't get next image to render to from the swapchain" );
        return false;
    }

    ctx->current_image_index = current_image;

    CommandBuffer cmdBuffer = ctx->swapchain_info.graphics_cmd_buffers_per_image.data[current_image];

    cmdBuffer.Reset();
    cmdBuffer.Begin( false, false, false );

    ctx->renderPass.begin( &ctx->renderPass, &cmdBuffer );

    return true;
}

bool UpdateTexture( VulkanContext* context, Shader* in_shader )
{
    uint32_t currentIndex = context->current_image_index;
    CommandBuffer currentCmdBuffer = context->swapchain_info.graphics_cmd_buffers_per_image.data[currentIndex];
    VkDescriptorSet currentDescriptor = in_shader->descriptor_sets[currentIndex].data[1];

    VkDescriptorImageInfo image_info = {};
    image_info.imageView = context->default_texture.view;
    image_info.sampler = context->default_sampler;
    image_info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writeDescriptor = {};
    writeDescriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptor.dstSet = currentDescriptor;   // "dst" stands for "descriptor" here 
    writeDescriptor.dstBinding = 0;               // "dst" stands for "descriptor" here
    writeDescriptor.dstArrayElement = 0;          // "dst" stands for "descriptor" here
    writeDescriptor.descriptorCount = 1;
    writeDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescriptor.pImageInfo = &image_info;

    vkUpdateDescriptorSets( context->logicalDeviceInfo.handle, 1, &writeDescriptor, 0, nullptr );

    return true;
}


bool UpdateGlobalState( BackendRenderer* in_backend, Matrix4x4 projection, Matrix4x4 view, float ambiant, uint32_t mode )
{
    VulkanContext* ctx = (VulkanContext*) in_backend->user_data;

    CommandBuffer current_cmd = ctx->swapchain_info.graphics_cmd_buffers_per_image.data[ctx->current_image_index];

    // NOTE : the projection and view are passed by copy because the frame might not be done
    ctx->default_shader.global_UBO.projection = projection;
    ctx->default_shader.global_UBO.view = view;

    Shader::UpdateGlobalBuffer( ctx, ctx->default_shader.global_UBO, &ctx->default_shader );

    return true;
}

/// Once we reach this function , the image_index represents the current image that's we're working on to send to presentation
bool EndFrame( BackendRenderer* in_backend, RendererContext rendererContext )
{
    VulkanContext* ctx = (VulkanContext*) in_backend->user_data;
    CommandBuffer cmdBuffer = ctx->swapchain_info.graphics_cmd_buffers_per_image.data[ctx->current_image_index];

    // test code
    {
        Shader::Bind( ctx, &ctx->default_shader );

        GameState* state = &Global::app.game_app.game_state;

        float aspect = Global::platform.window.width / (float) Global::platform.window.height;
        Matrix4x4 proj = Matrix4x4::Perspective( 60, 0.1f, 100, aspect );

        Matrix4x4 cor_mat = Matrix4x4( { 1,0,0,0 }, { 0,-1,0,0 }, { 0,0,-1,0 }, { 0,0,0,1 } );
        Matrix4x4 tra_mat = Matrix4x4::Translate( state->camera_position );
        Matrix4x4 rot_mat = Matrix4x4::Rotate( state->camera_rotation );
        Matrix4x4 scl_mat = Matrix4x4::Scale( Vector3( 1, 1, -1 ) );
        Matrix4x4 nonInvView = tra_mat * rot_mat;
        Matrix4x4 view = Matrix4x4::Inverse( nonInvView ) * scl_mat;

        {
            // vulkan considers (0,0) to be the upper-left corner
            // to get "standanrdized" zero point , we set the the center to be (bottom-left)
            // hence why the y == height and height = -height
            VkViewport viewport = {};
            viewport.x = 0;
            viewport.y = (float) ctx->frameBufferSize.y;
            viewport.width = (float) ctx->frameBufferSize.x;
            viewport.height = -(float) ctx->frameBufferSize.y;
            viewport.maxDepth = 1;
            viewport.minDepth = 0;

            VkRect2D scissor = { };
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = ctx->frameBufferSize.x;
            scissor.extent.height = ctx->frameBufferSize.y;

            vkCmdSetViewport( cmdBuffer.handle, 0, 1, &viewport );
            vkCmdSetScissor( cmdBuffer.handle, 0, 1, &scissor );
        }

        uint32_t currentImageIndex = ctx->current_image_index;

        UpdateTexture( ctx, &ctx->default_shader );
        UpdateGlobalState( in_backend, proj, view, 1, 0 );

        DArray<VkDescriptorSet>* currSet = &ctx->default_shader.descriptor_sets[currentImageIndex];
        for ( size_t i = 0; i < currSet->size; ++i )
        {
            VkDescriptorSet* curr = &currSet->data[i];
            vkCmdBindDescriptorSets( cmdBuffer.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx->default_shader.pipeline.layout, i, 1, curr, 0, nullptr );
        }

        VkDeviceSize pos_offsets[1] = { 0 };
        VkDeviceSize tex_offsets[1] = { 12 };
        vkCmdBindVertexBuffers( cmdBuffer.handle, 0, 1, &ctx->vertexBuffer.handle, (VkDeviceSize*) pos_offsets );
        vkCmdBindVertexBuffers( cmdBuffer.handle, 1, 1, &ctx->vertexBuffer.handle, (VkDeviceSize*) tex_offsets );
        vkCmdBindIndexBuffer( cmdBuffer.handle, ctx->indexBuffer.handle, 0, VkIndexType::VK_INDEX_TYPE_UINT32 );
        vkCmdDrawIndexed( cmdBuffer.handle, 6, 1, 0, 0, 0 );
    }

    ctx->renderPass.end( &ctx->renderPass, &cmdBuffer );
    cmdBuffer.End();

    // NOTE : a very important detail to understand here is that we're reusing the Fence used to signal "CommandBuffer finished" to also signal "Frame Present"
    // this might seem confusing at first , but since "Present" always happens after "CommandBuffer finished"
    // we don't really need two different Fences , we can use the Fence to signal "CommandBuffer finished" first
    // then , rest the fence and reuse it to signal "Frame Present"

    // the previous frame might be using the same image as the current frame.
    // if so , we wait it's fence
    Fence* inFlightFence = ctx->swapchain_info.in_flight_fence_per_image.data[ctx->current_image_index];

    if ( inFlightFence != VK_NULL_HANDLE )
    {
        if ( !inFlightFence->Wait( ctx, UINT64_MAX ) )
        {
            Global::logger.Error( "Couldn't wait previous frame's fence" );
        }
    }

    // now the image is free
    // all this does is declare that image[currentImageIndex] is being used by frame[currentFrame] and the fence[currentFrame]
    // we declare that image[0] for example, is now associated with the command using fence[2] for example
    // (notice how the frame_index doesn't necessarily have to match the image_index)
    // once the fence is signaled , we know that the image is free to be reused

    // so , here  we take the the address of the Fence that we just waited for
    Fence* submit_fence = &ctx->swapchain_info.cmd_buffer_done_execution_per_frame.data[ctx->current_frame];

    // reset it
    submit_fence->Reset( ctx );

    // then reuse it as "in flight" fence
    ctx->swapchain_info.in_flight_fence_per_image.data[ctx->current_image_index] = submit_fence;

    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &cmdBuffer.handle;

    // signaled when the queue is complete
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &ctx->swapchain_info.finished_rendering_semaphores.data[ctx->current_frame];

    // wait semaphore to make sure that the operation cannot begin until the image is available
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &ctx->swapchain_info.image_presentation_complete_semaphores.data[ctx->current_frame];

    VkPipelineStageFlags pipelineFlags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    info.pWaitDstStageMask = &pipelineFlags[0];

    // this is an async (non-blocking call) that returns immidiately
    // to know if the sumbission is done on the GPU side , we will need to check the Fence passed
    // from the docs : fence is an optional handle to a fence to be signaled once all submitted command buffers have completed execution
    vkQueueSubmit( ctx->physicalDeviceInfo.queuesInfo.graphicsQueue, 1, &info, submit_fence->handle );

    cmdBuffer.UpdateSubmitted();

    // note : accordring to the spec
    // presentation requests sent in a particular queue are always performed in order
    // that way , we can be sure that present order is always 0,1,2,0,1,2,0,1,2....
    ctx->swapchain_info.Present
    (
        ctx,
        // wait the finish rendering semaphore
        // this will make sure that we send the image for presentation AFTER all the rendering commands are done drawing to it
        ctx->swapchain_info.finished_rendering_semaphores.data[ctx->current_frame],
        &ctx->current_image_index
    );

    // increments and loop frame count
    ctx->current_frame = (ctx->current_frame + 1) % ctx->swapchain_info.maxFramesInFlight;

    return true;
}



bool Destroy( BackendRenderer* in_backend )
{
    VulkanContext* ctx = (VulkanContext*) in_backend->user_data;
    Global::event_system.Unlisten<WindowResizeEvent>( HandleWindowResize );

    vkDeviceWaitIdle( ctx->logicalDeviceInfo.handle );

    Buffer::Destroy( ctx, &ctx->vertexBuffer );
    Buffer::Destroy( ctx, &ctx->indexBuffer );

    Shader::Destroy( ctx, &ctx->default_shader );

    DescriptorManager::Destroy( &ctx->descriptor_manager );

    vkDestroyCommandPool( ctx->logicalDeviceInfo.handle, ctx->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, ctx->allocator );

    ctx->renderPass.on_destroy( &ctx->renderPass );
    vkDestroyRenderPass( ctx->logicalDeviceInfo.handle, ctx->renderPass.handle, ctx->allocator );

    SwapchainInfo::Destroy( ctx, &ctx->swapchain_info );

    vkDestroySurfaceKHR( ctx->vulkanInstance, ctx->surface, ctx->allocator );

    vkDestroyDevice( ctx->logicalDeviceInfo.handle, ctx->allocator );

    return true;
}

void VulkanBackendRenderer::Create( BackendRenderer* out_renderer )
{
    out_renderer->startup = Startup;
    out_renderer->resize = Resize;
    out_renderer->start_frame = StartFrame;
    out_renderer->end_frame = EndFrame;
    out_renderer->update_global_state = UpdateGlobalState;
    out_renderer->destroy = Destroy;
}

#include "VulkanBackendRenderer.h"
#include "../../../../Defines/Defines.h"
#include <vulkan/vulkan_win32.h>
#include "../../../../Platform/Types/Win32/Platform/Win32Platform.h"
#include "../../../../Platform/Types/Win32/Window/Win32Window.h"
#include <format>
#include "../../../../Maths/Maths.h"
#include "../../../Frontend/Texture/Texture.h"
#include "../../../../Maths/Rect.h"
#include "../../../../Maths/Color.h"
#include "../Context/CommandBuffer.h"
#include "../../../Frontend/Shader/Shader.h"
#include "../../../Frontend/Buffer/Buffer.h"
#include "../../../../Maths/Vector3.h"


const char* INSATNCE_EXTENSION[3] =
{
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(_WIN32)
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#if defined(_DEBUG)
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};
const char* VK_LAYERS[1] = { "VK_LAYER_KHRONOS_validation" };

const char* DEVICE_EXTENSIONS[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
VKAPI_ATTR VkBool32 VulkanBackendRenderer::DebugCallback (
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* userData )
{
    switch ( severity )
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        {
            Logger::Error ( callbackData->pMessage );
            break;
        }

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        {
            Logger::Log ( callbackData->pMessage );
            break;
        }

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        {
            Logger::Info ( callbackData->pMessage );
            break;
        }

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        {
            Logger::Warning ( callbackData->pMessage );
            break;
        }
    }

    return VK_FALSE;
}


bool HasExtensions ( VkExtensionProperties* extensionsArr, uint32_t extensionCount, const char** requiredExt, uint32_t requiredCount )
{
    for ( uint32_t i = 0; i < requiredCount; i++ )
    {
        auto req = requiredExt[i];

        bool foundExt = false;

        for ( uint32_t j = 0; j < extensionCount; j++ )
        {
            auto ext = extensionsArr[j];
            if ( strcmp ( ext.extensionName, req ) == 0 )
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

bool VulkanBackendRenderer::CreateSurface ( VulkanContext* context, VkSurfaceKHR* surface )
{
#ifdef _WIN32

    Win32Platform* win32plat = dynamic_cast<Win32Platform*>(platform);
    Win32Window* win32window = dynamic_cast<Win32Window*>(platform->window);

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = win32plat->processHandle;
    createInfo.hwnd = win32window->windowHandle;

    VkResult result = vkCreateWin32SurfaceKHR ( context->vulkanInstance, &createInfo, nullptr, surface );

    return result == VK_SUCCESS;

#else
    return false;
#endif
}

VulkanBackendRenderer::VulkanBackendRenderer ( Application* app, Platform* platform )
{
    this->application = app;
    this->platform = platform;
}

struct PhysicalDeviceRequirements
{
public:
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    bool samplerAnisotropy;
    bool discreteGPU;
};

struct LogicalDeviceRequirements
{
public:
    bool samplerAnisotropy;
};

bool CreateLogicalDevice ( VkInstance vkInstance, VkSurfaceKHR surface, PhysicalDeviceInfo* physicalDeviceinfo, LogicalDeviceRequirements* requirements, const VkAllocationCallbacks* allocator, VkDevice* handle )
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties ( physicalDeviceinfo->handle, &queueFamilyCount, nullptr );

    std::vector<int> queuesNeededPerFamily = std::vector<int> ( queueFamilyCount, 0 );

    // for now , if multiple requirement share a queue index , only one queue will used for them
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.computeQueueFamilyIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.graphicsQueueIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.presentQueueFamilyIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queuesInfo.transferQueueIndex]++;

    std::vector< VkDeviceQueueCreateInfo> queueCreationInfos = std::vector< VkDeviceQueueCreateInfo> ();


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
        float* queuePrio = new float[createQueueInfo.queueCount] ();

        createQueueInfo.pQueuePriorities = queuePrio;
        queueCreationInfos.push_back ( createQueueInfo );
    }

    // todo : we can fill this struct to request more device features
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = requirements->samplerAnisotropy;

    VkDeviceCreateInfo createDeviceInfo = {};
    createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createDeviceInfo.queueCreateInfoCount = queueCreationInfos.size ();
    createDeviceInfo.pQueueCreateInfos = queueCreationInfos.data ();
    createDeviceInfo.pEnabledFeatures = &deviceFeatures;
    createDeviceInfo.enabledExtensionCount = 1;
    const char* extension_names = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    createDeviceInfo.ppEnabledExtensionNames = &extension_names;

    // these 2 features are deprecated/ignored
    createDeviceInfo.enabledLayerCount = 0;
    createDeviceInfo.ppEnabledLayerNames = nullptr;

    VkResult result = vkCreateDevice ( physicalDeviceinfo->handle, &createDeviceInfo, allocator, handle );

    // free the prio queues
    for ( uint32_t i = 0; i < queueCreationInfos.size (); ++i )
    {
        delete[] queueCreationInfos[i].pQueuePriorities;
    }

    if ( result != VK_SUCCESS )
    {
        return false;
    }

    vkGetDeviceQueue ( *handle, physicalDeviceinfo->queuesInfo.graphicsQueueIndex, 0, &physicalDeviceinfo->queuesInfo.graphicsQueue );
    vkGetDeviceQueue ( *handle, physicalDeviceinfo->queuesInfo.computeQueueFamilyIndex, 0, &physicalDeviceinfo->queuesInfo.computeQueue );
    vkGetDeviceQueue ( *handle, physicalDeviceinfo->queuesInfo.presentQueueFamilyIndex, 0, &physicalDeviceinfo->queuesInfo.presentQueue );
    vkGetDeviceQueue ( *handle, physicalDeviceinfo->queuesInfo.transferQueueIndex, 0, &physicalDeviceinfo->queuesInfo.transferQueue );

    return true;
}

bool PhysicalDeviceHasSwapchainSupport ( VkPhysicalDevice handle, VkSurfaceKHR surface, SwapchainSupportInfo* outSwapchainInfo )
{
    // capabilities
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR ( handle, surface, &capabilities );

    // surface formats
    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR ( handle, surface, &formatsCount, nullptr );
    std::vector<VkSurfaceFormatKHR> formats = std::vector<VkSurfaceFormatKHR> ( formatsCount );
    vkGetPhysicalDeviceSurfaceFormatsKHR ( handle, surface, &formatsCount, formats.data () );

    // present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR ( handle, surface, &presentModeCount, nullptr );
    std::vector<VkPresentModeKHR> presentModes = std::vector<VkPresentModeKHR> ( presentModeCount );
    vkGetPhysicalDeviceSurfacePresentModesKHR ( handle, surface, &presentModeCount, presentModes.data () );

    // todo : here we add a check for device extensions

    outSwapchainInfo->capabilities = capabilities;
    outSwapchainInfo->presentModes = presentModes;
    outSwapchainInfo->surfaceFormats = formats;

    return true;
}

bool CreatePhysicalDevice ( Platform* platform, VkInstance vkInstance, VkSurfaceKHR* surface, PhysicalDeviceInfo* outDeviceInfo )
{

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices ( vkInstance, &physicalDeviceCount, nullptr );

    std::vector<PhysicalDeviceRequirements> infos = std::vector<PhysicalDeviceRequirements> ( physicalDeviceCount );

    std::vector <VkPhysicalDevice> physicalDevices = std::vector<VkPhysicalDevice> ( physicalDeviceCount );

    vkEnumeratePhysicalDevices ( vkInstance, &physicalDeviceCount, physicalDevices.data () );


    PhysicalDeviceRequirements requirements;
    requirements.graphics = true;
    requirements.present = true;
    requirements.compute = true;
    requirements.transfer = true;
    requirements.samplerAnisotropy = true;
    requirements.discreteGPU = true;

    int maxScore = 0;

    Logger::Log ( "Selecting physical device ...." );
    for ( uint32_t i = 0; i < physicalDeviceCount; ++i )
    {

        VkPhysicalDevice currPhysicalDevice = physicalDevices[i];
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memory;

        vkGetPhysicalDeviceProperties ( currPhysicalDevice, &props );
        vkGetPhysicalDeviceFeatures ( currPhysicalDevice, &features );
        vkGetPhysicalDeviceMemoryProperties ( currPhysicalDevice, &memory );

        Logger::Log ( std::format ( "> Evaluating device : {}", props.deviceName ).c_str () );

        // discrete
        if ( (requirements.discreteGPU) && (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) )
        {
            Logger::Error ( "This device is not a discrete GPU , skippping ..." );
            continue;
        }

        if ( (requirements.samplerAnisotropy) && (features.samplerAnisotropy == VK_FALSE) )
        {
            Logger::Error ( "This device does not support sampler anisotropy , skippping ..." );
            continue;
        }

        // queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties ( currPhysicalDevice, &queueFamilyCount, nullptr );

        std::vector<VkQueueFamilyProperties> queueProps = std::vector<VkQueueFamilyProperties> ( queueFamilyCount );
        vkGetPhysicalDeviceQueueFamilyProperties ( currPhysicalDevice, &queueFamilyCount, queueProps.data () );

        std::vector<float> presentScore = std::vector<float> ( queueFamilyCount );
        std::vector<float> graphicsScore = std::vector<float> ( queueFamilyCount );
        std::vector<float> transferScore = std::vector<float> ( queueFamilyCount );
        std::vector<float> computeScore = std::vector<float> ( queueFamilyCount );

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
            vkGetPhysicalDeviceSurfaceSupportKHR ( currPhysicalDevice, i, *surface, &presentSupported );

            currScore = 0;
            if ( presentSupported == VK_TRUE )
            {
                currScore = (float) 1 / currQueue.queueCount;
            }

            presentScore[i] = currScore;
        }

        // we use this to check if a certain feature is not present at all
        std::vector<float> emptyScore = std::vector<float> ( queueFamilyCount, 0 );

        if ( (requirements.present) && presentScore == emptyScore )
        {
            Logger::Error ( "This device does't support present , skipping ..." );
            continue;
        }

        if ( (requirements.graphics) && emptyScore == graphicsScore )
        {
            Logger::Error ( "This device does't support graphics , skipping ..." );
            continue;
        }
        if ( (requirements.transfer) && emptyScore == transferScore )
        {
            Logger::Error ( "This device does't support transfer , skipping ..." );
            continue;
        }
        if ( (requirements.compute) && emptyScore == computeScore )
        {
            Logger::Error ( "This device does't support compute , skipping ..." );
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
        if ( !PhysicalDeviceHasSwapchainSupport ( currPhysicalDevice, *surface, &swapchainSupportInfo ) )
        {
            Logger::Error ( "This device doesn't support swapchain , skipping ..." );
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

bool CreateGraphicsCommandPools ( VulkanContext* context )
{
    VkCommandPoolCreateInfo graphicsPoolCreate = {};
    graphicsPoolCreate.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsPoolCreate.queueFamilyIndex = context->physicalDeviceInfo.queuesInfo.graphicsQueueIndex;
    graphicsPoolCreate.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;


    vkCreateCommandPool ( context->logicalDeviceInfo.handle, &graphicsPoolCreate, context->allocator, &context->physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool );
    return true;
}

bool UploadDataRange ( VulkanContext* context, Memory* memory, VkCommandPool pool, Fence fence, VkQueue queue, Buffer* inBuffer, uint32_t offset, uint32_t size, void* inDataPtr )
{
    // first , we create a host-visible staging buffer to upload the data to in
    // then we mark it as the source of the transfer
    VkBufferUsageFlags usage = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    Buffer stagingBuffer = {};

    BufferDescriptor bufferDesc = {};
    bufferDesc.size = size;
    bufferDesc.memoryPropertyFlags = usage;
    bufferDesc.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Buffer::Create ( context, bufferDesc, true, &stagingBuffer );

    Buffer::Load ( context, memory, 0, size, inDataPtr, 0, &stagingBuffer );

    Buffer::Copy ( context, pool, fence, queue, &stagingBuffer, 0, inBuffer, offset, size );

    Buffer::Destroy ( context, &stagingBuffer );

    return true;
}


void HandleWindowResize ( WindowResizeEvent  evt )
{
    evt.app->renderer.Resize ( evt.dimensions.x, evt.dimensions.y );
}

bool VulkanBackendRenderer::Startup ()
{
    this->context = VulkanContext ();
    this->context.allocator = nullptr;

    this->context.frameBufferSize.x = this->application->platform->window->width;
    this->context.frameBufferSize.y = this->application->platform->window->height;

    this->application->gameEventSystem.Listen<WindowResizeEvent> ( Action<WindowResizeEvent> ( HandleWindowResize ) );

    // check for extensions
    uint32_t instance_extCount = 0;
    vkEnumerateInstanceExtensionProperties ( nullptr, &instance_extCount, nullptr );
    VkExtensionProperties* instance_exts = new VkExtensionProperties[instance_extCount];

    vkEnumerateInstanceExtensionProperties ( nullptr, &instance_extCount, instance_exts );

    Logger::Log ( (std::string ( "Instance extensions available : " ) + std::to_string ( instance_extCount )).c_str () );
    for ( uint32_t i = 0; i < instance_extCount; ++i )
    {
        Logger::Log ( instance_exts[i].extensionName );
    }

    delete[] instance_exts;

    // info about our app
    VkApplicationInfo appInfo {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Defines::EngineName;
    appInfo.pEngineName = Defines::EngineName;

    appInfo.applicationVersion = VK_MAKE_VERSION ( 1, 0, 0 );
    appInfo.engineVersion = VK_MAKE_VERSION ( 1, 0, 0 );
    appInfo.apiVersion = VK_API_VERSION_1_0;


    // creation info
    VkInstanceCreateInfo createInfo {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    // layers
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = VK_LAYERS;

    // extensions
    createInfo.enabledExtensionCount = 3;
    createInfo.ppEnabledExtensionNames = INSATNCE_EXTENSION;

    VkResult result = vkCreateInstance ( &createInfo, nullptr, &context.vulkanInstance );

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
    debugInfoCreate.pfnUserCallback = &VulkanBackendRenderer::DebugCallback;

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr ( this->context.vulkanInstance, "vkCreateDebugUtilsMessengerEXT" );
    func ( context.vulkanInstance, &debugInfoCreate, context.allocator, &context.debugMessenger );

    Logger::NewLine ();
    Logger::NewLine ();

    Logger::Log ( "Vulkan Debugger created" );

#endif
    Logger::NewLine ();
    Logger::NewLine ();


    Logger::Log ( "Logical extensions needed :" );
    for ( const auto& ext : DEVICE_EXTENSIONS )
    {
        Logger::Log ( ext );
    }

    // surface

    if ( !CreateSurface ( &context, &context.surface ) )
    {
        Logger::Error ( "Couldn't create Vulkan Surface ..." );
        return false;
    }

    Logger::Log ( "Vulkan surface created" );


    Logger::NewLine ();

    PhysicalDeviceInfo physicalDeviceInfo = {};
    if ( !CreatePhysicalDevice ( this->platform, this->context.vulkanInstance, &this->context.surface, &physicalDeviceInfo ) )
    {
        Logger::Error ( "Couldn't create Physical device ..." );
        return false;
    }

    context.physicalDeviceInfo = physicalDeviceInfo;

    Logger::NewLine ();

    Logger::Info ( "> Selected physical device" );

    Logger::Info ( std::format ( "Name : {} ", physicalDeviceInfo.physicalDeviceProperties.deviceName ).c_str () );

    std::string deviceType = {};
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

    Logger::Info ( std::format ( "GPU Type : {} ", deviceType ).c_str () );

    Logger::Info ( std::format ( "GPU Driver version : {}.{}.{}",
                   VK_API_VERSION_MAJOR ( physicalDeviceInfo.physicalDeviceProperties.driverVersion ),
                   VK_API_VERSION_MINOR ( physicalDeviceInfo.physicalDeviceProperties.driverVersion ),
                   VK_API_VERSION_PATCH ( physicalDeviceInfo.physicalDeviceProperties.driverVersion )
    ).c_str () );


    Logger::Info ( std::format ( "Vulkan API version : {}.{}.{} ",
                   VK_API_VERSION_MAJOR ( physicalDeviceInfo.physicalDeviceProperties.apiVersion ),
                   VK_API_VERSION_MINOR ( physicalDeviceInfo.physicalDeviceProperties.apiVersion ),
                   VK_API_VERSION_PATCH ( physicalDeviceInfo.physicalDeviceProperties.apiVersion )
    ).c_str () );

    Logger::NewLine ();

    LogicalDeviceRequirements logicalRequirements = {};
    logicalRequirements.samplerAnisotropy = true;

    VkDevice handle = {};
    if ( !CreateLogicalDevice ( context.vulkanInstance, context.surface, &context.physicalDeviceInfo, &logicalRequirements, context.allocator, &handle ) )
    {
        Logger::Error ( "Couldn't create logical device ...." );
        return false;
    }

    context.logicalDeviceInfo.handle = handle;

    Logger::Info ( "Logical device and queue created" );

    // create graphics command pool
    if ( !CreateGraphicsCommandPools ( &context ) )
    {
        Logger::Error ( "Couldn't create command pools ...." );
        return false;
    }

    Logger::Info ( "Command pools created" );

    // create swapshain
    SwapchainCreateDescription swapchainDesc = {};
    swapchainDesc.width = platform->window->width;
    swapchainDesc.height = platform->window->height;
    swapchainDesc.imagesCount = 3;

    if ( !SwapchainInfo::Create ( &context, swapchainDesc, &context.swapchainInfo ) )
    {
        Logger::Error ( "Couldn't create swapchain ...." );
        return false;
    }

    Logger::Info ( "Swapchain created" );

    // create renderpass
    Rect rect = {};
    rect.x = 0;
    rect.y = 0;
    rect.width = platform->window->width;
    rect.height = platform->window->height;

    Color clearColor = {};
    clearColor.r = 0;
    clearColor.g = 0;
    clearColor.b = 0.2;
    clearColor.a = 1;

    if ( !Renderpass::Create ( &context, rect, clearColor, 1, 0, &context.renderPass ) )
    {
        Logger::Error ( "Couldn't create renderpass ...." );
        return false;
    }

    Logger::Info ( "Renderpass created" );

    if ( !context.swapchainInfo.CreateFrameBuffers ( &context ) )
    {
        Logger::Error ( "Couldn't create frame buffers ...." );
        return false;
    }

    Logger::Info ( "Frame buffers created" );


    if ( !Shader::Create ( &context, platform->filesystem, &context.defaultShader ) )
    {
        return false;
    }

    Logger::Info ( "Default shader created" );

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
        bufferDesc.size = sizeof ( Vector3 ) * 1024 * 1024;
        bufferDesc.usage = usage;

        if ( !Buffer::Create ( &context, bufferDesc, true, &context.vertexBuffer ) )
        {
            Logger::Error ( "Couldn't create vertex buffer ...." );
            return false;
        }

        context.geometryVertexOffset = 0;
        Logger::Info ( "Vertex buffer created" );
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
        bufferDesc.size = sizeof ( uint32_t ) * 1024 * 1024;
        bufferDesc.usage = usage;

        if ( !Buffer::Create ( &context, bufferDesc, true, &context.indexBuffer ) )
        {
            Logger::Error ( "Couldn't create index buffer ...." );
            return false;
        }

        context.indexBufferSize = bufferDesc.size;
        Logger::Info ( "Index buffer created" );
    }

    // test mesh
    {
        const uint32_t vertCount = 3;

        Vector3 vertPositions[3] = {
            Vector3 ( -0.5f , 0.5f , 0.0 ),
            Vector3 ( 0.5f , 0.5f , 0.0 ),
            Vector3 ( 0.0f , -0.5f , 0.0 ),

        };

        uint32_t vertIndicies[3] = {
            2 ,
            1 ,
            0 ,

        };

        uint32_t vertexSize = sizeof ( Vector3 ) * vertCount;
        uint32_t indiciesSize = sizeof ( uint32_t ) * vertCount;

        UploadDataRange ( &context, application->platform->memory, context.physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, {}, context.physicalDeviceInfo.queuesInfo.graphicsQueue, &context.vertexBuffer, 0, vertexSize, vertPositions );
        UploadDataRange ( &context, application->platform->memory, context.physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, {}, context.physicalDeviceInfo.queuesInfo.graphicsQueue, &context.indexBuffer, 0, indiciesSize, vertIndicies );
    }

    return true;
}


void VulkanBackendRenderer::Resize ( uint32_t width, uint32_t height )
{
    this->context.frameBufferSize.x = width;
    this->context.frameBufferSize.y = height;
    this->context.frameBufferSizeCurrentGeneration++;
}

bool VulkanBackendRenderer::StartFrame ( RendererContext rendererContext )
{
    LogicalDeviceInfo device = context.logicalDeviceInfo;

    if ( context.recreateSwapchain )
    {
        VkResult result = vkDeviceWaitIdle ( device.handle );

        if ( result != VkResult::VK_SUCCESS )
        {
            Logger::Error ( "Couldn't wait device idle" );
        }
        return false;
    }

    // if we need to recreate the swapchain
    if ( context.frameBufferSizeCurrentGeneration != context.frameBufferSizeLastGeneration )
    {
        VkResult result = vkDeviceWaitIdle ( device.handle );

        if ( result != VkResult::VK_SUCCESS )
        {
            Logger::Error ( "Couldn't wait device idle" );
            return false;
        }

        SwapchainCreateDescription desc = {};
        desc.width = context.frameBufferSize.x;
        desc.height = context.frameBufferSize.y;
        desc.imagesCount = 3;
        SwapchainInfo::Recreate ( &context, desc, &context.swapchainInfo );

        return false;
    }

    // first we get the index of the last frame index
    uint32_t currentFrame = context.currentFrame;

    // we wait until the previous frame is done with it's command buffers
    if ( !context.swapchainInfo.cmdBufferSumitFencePerFrameIndex[currentFrame].Wait ( &context, UINT64_MAX ) )
    {
        return false;
    }

    // we ask the swapchain to get us the index of an image that we can render to  
    uint32_t currentImage = {};
    if ( !context.swapchainInfo.AcquireNextImageIndex ( &context, UINT64_MAX, context.swapchainInfo.imageAvailableSemaphore[currentFrame], nullptr, &currentImage ) )
    {
        Logger::Error ( "Couldn't get image to render to" );
        return false;
    }

    context.currentImageIndex = currentImage;

    CommandBuffer cmdBuffer = context.swapchainInfo.graphicssCommandBuffers[currentImage];

    cmdBuffer.Reset ();
    cmdBuffer.Begin ( false, false, false );


    // vulkan considers (0,0) to be the upper-left corner
    // to get "standanrdized" zero point
    // we set the the center to be (bottom-left)
    // hence why the y == height and height = -height
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = context.frameBufferSize.y;
    viewport.width = context.frameBufferSize.x;
    viewport.height = -context.frameBufferSize.y;
    viewport.maxDepth = 1;
    viewport.minDepth = 0;

    VkRect2D scissor = { };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = context.frameBufferSize.x;
    scissor.extent.height = context.frameBufferSize.y;

    vkCmdSetViewport ( cmdBuffer.handle, 0, 1, &viewport );
    vkCmdSetScissor ( cmdBuffer.handle, 0, 1, &scissor );

    context.renderPass.area.width = context.frameBufferSize.x;
    context.renderPass.area.height = context.frameBufferSize.y;

    context.renderPass.Begin ( &context, &cmdBuffer, &context.swapchainInfo.frameBuffers[currentImage] );

    // test code
    {
        Shader::Bind ( &context, &context.defaultShader );

        VkDeviceSize offsets[1] = { 0 };
        vkCmdBindVertexBuffers ( cmdBuffer.handle, 0, 1, &context.vertexBuffer.handle, (VkDeviceSize*) offsets );
        vkCmdBindIndexBuffer ( cmdBuffer.handle, context.indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32 );
        //vkCmdDrawIndexed ( cmdBuffer.handle, 3, 1, 0, 0, 0 );
        vkCmdDraw ( cmdBuffer.handle, 3, 1, 0, 0 );
    }

    return true;
}

bool VulkanBackendRenderer::EndFrame ( RendererContext rendererContext )
{
    CommandBuffer cmdBuffer = context.swapchainInfo.graphicssCommandBuffers[context.currentImageIndex];
    context.renderPass.End ( &context, &cmdBuffer );
    cmdBuffer.End ();

    // the previous frame might be using the same image as the current frame.
    // if so , we wait it's fence
    Fence* inFlightFence = context.swapchainInfo.fencePtrAssociatedPerImageIndex[context.currentImageIndex];

    if ( inFlightFence != VK_NULL_HANDLE )
    {
        if ( !inFlightFence->Wait ( &context, UINT64_MAX ) )
        {
            Logger::Error ( "Wait previous frame's fence" );
        }
    }

    // now the image is free
    // all this does is declare that image[currentImageIndex] is being used by frame[currentFrame] and its fence
    // we declare that THIS image is now associated with the command using THIS fence
    // once the fence is signaled , we know that the image is free to be reused
    context.swapchainInfo.fencePtrAssociatedPerImageIndex[context.currentImageIndex] = &context.swapchainInfo.cmdBufferSumitFencePerFrameIndex[context.currentFrame];

    context.swapchainInfo.cmdBufferSumitFencePerFrameIndex[context.currentFrame].Reset ( &context );


    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &cmdBuffer.handle;

    // signaled when the queue is complete
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &context.swapchainInfo.finishedRenderingSemaphore[context.currentFrame];

    // wait semaphore to make sure that the operation cannot begin until the image is available
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &context.swapchainInfo.imageAvailableSemaphore[context.currentFrame];

    VkPipelineStageFlags pipelineFlags[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    info.pWaitDstStageMask = &pipelineFlags[0];

    // this is an async (non-blocking call) that returns immidiately , to know if the sumbission is done on the GPU side , we will need to check the Fence passed
    vkQueueSubmit ( context.physicalDeviceInfo.queuesInfo.graphicsQueue, 1, &info, context.swapchainInfo.cmdBufferSumitFencePerFrameIndex[context.currentFrame].handle );

    cmdBuffer.UpdateSubmitted ();

    // note : accordring to the spec
    // presentation requests sent to a particular queue are always performed in order
    // that way , we can be sure that present order is always 0,1,2,0,1,2,0,1,2....
    context.swapchainInfo.Present
    (
        &context,
        // wait the finish rendering semaphore
        context.swapchainInfo.finishedRenderingSemaphore[context.currentFrame],
        &context.currentImageIndex
    );

    return true;
}

void VulkanBackendRenderer::Destroy ()
{
    this->application->gameEventSystem.Unlisten<WindowResizeEvent> ( Action<WindowResizeEvent> ( HandleWindowResize ) );

    vkDeviceWaitIdle ( context.logicalDeviceInfo.handle );

    Buffer::Destroy ( &context, &context.vertexBuffer );
    Buffer::Destroy ( &context, &context.indexBuffer );

    Shader::Destroy ( &context, &context.defaultShader );

    SwapchainInfo::Clear ( &context, &context.swapchainInfo );

    vkDestroyCommandPool ( context.logicalDeviceInfo.handle, context.physicalDeviceInfo.commandPoolsInfo.graphicsCommandPool, context.allocator );

    Renderpass::Destroy ( &context, &context.renderPass );

    SwapchainInfo::Destroy ( &context, &context.swapchainInfo );

    vkDestroySurfaceKHR ( context.vulkanInstance, context.surface, context.allocator );

    vkDestroyDevice ( context.logicalDeviceInfo.handle, context.allocator );
}

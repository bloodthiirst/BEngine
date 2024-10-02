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
#include "../Renderpasses/UIRenderpass.h"
#include "../../AssetManager/ShaderAssetManager.h"
#include <Maths/Vector2.h>
#include "../Mesh/Vertex3D.h"

struct PhysicalDeviceRequirements
{
    bool graphics;
    bool present;
    bool compute;
    bool transfer;
    bool sampler_anisotropy;
    bool discrete_GPU;
};

struct LogicalDeviceRequirements
{
    bool samplerAnisotropy;
};

const char *INSATNCE_EXTENSION[] =
    {
        VK_KHR_SURFACE_EXTENSION_NAME,

#if defined(_WIN32)
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif

#if defined(_DEBUG)
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
};
const char *VK_LAYERS[] = {"VK_LAYER_KHRONOS_validation"};

const char *DEVICE_EXTENSIONS[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_MAINTENANCE1_EXTENSION_NAME};

VKAPI_ATTR VkBool32 DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
    void *userData)
{
    switch (severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        Global::logger.Error(callbackData->pMessage);
        break;
    }

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    {
        Global::logger.Log(callbackData->pMessage);
        break;
    }

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        Global::logger.Info(callbackData->pMessage);
        break;
    }

    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        Global::logger.Warning(callbackData->pMessage);
        break;
    }
    }

    return VK_FALSE;
}

bool HasExtensions(VkExtensionProperties *extensionsArr, uint32_t extensionCount, const char **requiredExt, uint32_t requiredCount)
{
    for (uint32_t i = 0; i < requiredCount; i++)
    {
        auto req = requiredExt[i];

        bool foundExt = false;

        for (uint32_t j = 0; j < extensionCount; j++)
        {
            auto ext = extensionsArr[j];
            if (strcmp(ext.extensionName, req) == 0)
            {
                foundExt = true;
                break;
            }
        }

        if (!foundExt)
            return false;
    }

    return true;
}

bool CreateSurface(VulkanContext *context, VkSurfaceKHR *surface)
{
#ifdef _WIN32

    Win32WindowState *win32plat = (Win32WindowState *)Global::platform.window.user_data;

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = win32plat->process_handle;
    createInfo.hwnd = win32plat->window_handle;

    VK_CHECK(vkCreateWin32SurfaceKHR(context->vulkan_instance, &createInfo, nullptr, surface), result);

    return result == VK_SUCCESS;

#else
    return false;
#endif
}

bool CreateLogicalDevice(VkInstance vkInstance, VkSurfaceKHR surface, PhysicalDeviceInfo *physicalDeviceinfo, LogicalDeviceRequirements *requirements, const VkAllocationCallbacks *allocator, VkDevice *handle)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDeviceinfo->handle, &queueFamilyCount, nullptr);

    Allocator alloc = STACK_ALLOC_ARRAY(int, queueFamilyCount);
    int *queuesNeededPerFamily = (int *)ALLOC(alloc, queueFamilyCount * sizeof(int));

    // for now , if multiple requirement share a queue index , only one queue will used for them
    queuesNeededPerFamily[physicalDeviceinfo->queues_info.computeQueueFamilyIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queues_info.graphicsQueueIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queues_info.presentQueueFamilyIndex]++;
    queuesNeededPerFamily[physicalDeviceinfo->queues_info.transferQueueIndex]++;

    Allocator alloc_create = STACK_ALLOC_ARRAY(VkDeviceQueueCreateInfo, queueFamilyCount);
    DArray<VkDeviceQueueCreateInfo> queueCreationInfos;
    DArray<VkDeviceQueueCreateInfo>::Create(queueFamilyCount, &queueCreationInfos, alloc_create);

    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        // no queues needed from this family
        if (queuesNeededPerFamily[i] == 0)
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
        float *queuePrio = Global::alloc_toolbox.HeapAlloc<float>(createQueueInfo.queueCount, true);

        createQueueInfo.pQueuePriorities = queuePrio;
        DArray<VkDeviceQueueCreateInfo>::Add(&queueCreationInfos, createQueueInfo);
    }

    // todo : we can fill this struct to request more device features
    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = requirements->samplerAnisotropy;

    VkDeviceCreateInfo createDeviceInfo = {};
    createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createDeviceInfo.queueCreateInfoCount = (uint32_t)queueCreationInfos.size;
    createDeviceInfo.pQueueCreateInfos = queueCreationInfos.data;

    createDeviceInfo.pEnabledFeatures = &deviceFeatures;
    createDeviceInfo.enabledExtensionCount = sizeof(DEVICE_EXTENSIONS) / sizeof(char *);
    createDeviceInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS;

    // these 2 features are deprecated/ignored
    createDeviceInfo.enabledLayerCount = 0;
    createDeviceInfo.ppEnabledLayerNames = nullptr;

    VK_CHECK(vkCreateDevice(physicalDeviceinfo->handle, &createDeviceInfo, allocator, handle), result);

    // free the prio queues
    for (uint32_t i = 0; i < queueCreationInfos.size; ++i)
    {
        Global::alloc_toolbox.HeapFree(queueCreationInfos.data[i].pQueuePriorities);
    }

    if (result != VK_SUCCESS)
    {
        return false;
    }

    vkGetDeviceQueue(*handle, physicalDeviceinfo->queues_info.graphicsQueueIndex, 0, &physicalDeviceinfo->queues_info.graphics_queue);
    vkGetDeviceQueue(*handle, physicalDeviceinfo->queues_info.computeQueueFamilyIndex, 0, &physicalDeviceinfo->queues_info.computeQueue);
    vkGetDeviceQueue(*handle, physicalDeviceinfo->queues_info.presentQueueFamilyIndex, 0, &physicalDeviceinfo->queues_info.presentQueue);
    vkGetDeviceQueue(*handle, physicalDeviceinfo->queues_info.transferQueueIndex, 0, &physicalDeviceinfo->queues_info.transferQueue);

    return true;
}

bool PhysicalDeviceHasSwapchainSupport(VkPhysicalDevice handle, VkSurfaceKHR surface, SwapchainSupportInfo *outSwapchainInfo)
{
    Allocator heap_alloc = Global::alloc_toolbox.heap_allocator;

    // capabilities
    VkSurfaceCapabilitiesKHR capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, &capabilities);

    // surface formats
    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &formatsCount, nullptr);
    DArray<VkSurfaceFormatKHR> formats;
    DArray<VkSurfaceFormatKHR>::Create(formatsCount, &formats, heap_alloc);
    vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, &formatsCount, formats.data);

    // present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &presentModeCount, nullptr);
    DArray<VkPresentModeKHR> presentModes;
    DArray<VkPresentModeKHR>::Create(presentModeCount, &presentModes, heap_alloc);

    vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, &presentModeCount, presentModes.data);

    // todo : here we add a check for device extensions

    outSwapchainInfo->capabilities = capabilities;
    outSwapchainInfo->presentModes = presentModes;
    outSwapchainInfo->surfaceFormats = formats;

    return true;
}

bool CreatePhysicalDevice(Platform *platform, VkInstance vkInstance, VkSurfaceKHR *surface, PhysicalDeviceInfo *outDeviceInfo)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, nullptr);

    ArenaCheckpoint checkpoint = Global::alloc_toolbox.GetArenaCheckpoint();
    // Allocator alloc_info = STACK_ALLOC_ARRAY( PhysicalDeviceRequirements, physicalDeviceCount );
    Allocator alloc_info = Global::alloc_toolbox.frame_allocator;
    PhysicalDeviceRequirements *infos = (PhysicalDeviceRequirements *)ALLOC(alloc_info, physicalDeviceCount * sizeof(PhysicalDeviceRequirements));

    // Allocator alloc_devices = STACK_ALLOC_ARRAY( VkPhysicalDevice, physicalDeviceCount );
    Allocator alloc_devices = Global::alloc_toolbox.frame_allocator;
    VkPhysicalDevice *physicalDevices = (VkPhysicalDevice *)ALLOC(alloc_devices, physicalDeviceCount * sizeof(VkPhysicalDevice));

    vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, physicalDevices);

    PhysicalDeviceRequirements requirements = {};
    requirements.graphics = true;
    requirements.present = true;
    requirements.compute = true;
    requirements.transfer = true;
    requirements.sampler_anisotropy = true;
    requirements.discrete_GPU = true;

    int maxScore = 0;

    Global::logger.Log("Selecting physical device ....");

    for (uint32_t i = 0; i < physicalDeviceCount; ++i)
    {
        VkPhysicalDevice currPhysicalDevice = physicalDevices[i];
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        VkPhysicalDeviceMemoryProperties memory;

        vkGetPhysicalDeviceProperties(currPhysicalDevice, &props);
        vkGetPhysicalDeviceFeatures(currPhysicalDevice, &features);
        vkGetPhysicalDeviceMemoryProperties(currPhysicalDevice, &memory);

        Global::logger.Log("> Evaluating device : {}", props.deviceName);

        // discrete
        if ((requirements.discrete_GPU) && (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU))
        {
            Global::logger.Error("This device is not a discrete GPU , skippping ...");
            continue;
        }

        if ((requirements.sampler_anisotropy) && (features.samplerAnisotropy == VK_FALSE))
        {
            Global::logger.Error("This device does not support sampler anisotropy , skippping ...");
            continue;
        }

        // queues
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, nullptr);

        Allocator alloc_queue = STACK_ALLOC_ARRAY(VkQueueFamilyProperties, queueFamilyCount);

        VkQueueFamilyProperties *queueProps = (VkQueueFamilyProperties *)ALLOC(alloc_queue, queueFamilyCount * sizeof(VkQueueFamilyProperties));

        vkGetPhysicalDeviceQueueFamilyProperties(currPhysicalDevice, &queueFamilyCount, queueProps);

        size_t mem_size = queueFamilyCount * sizeof(float);

        Allocator alloc_score = EmplaceAllocator::Create(_alloca(mem_size));
        Allocator alloc_graph = STACK_ALLOC(mem_size);
        Allocator alloc_transfer = STACK_ALLOC(mem_size);
        Allocator alloc_compute = STACK_ALLOC(mem_size);

        float *presentScore = (float *)ALLOC(alloc_score, mem_size);
        float *graphicsScore = (float *)ALLOC(alloc_graph, mem_size);
        float *transferScore = (float *)ALLOC(alloc_transfer, mem_size);
        float *computeScore = (float *)ALLOC(alloc_compute, mem_size);

        Global::platform.memory.mem_init(presentScore, mem_size);
        Global::platform.memory.mem_init(graphicsScore, mem_size);
        Global::platform.memory.mem_init(transferScore, mem_size);
        Global::platform.memory.mem_init(computeScore, mem_size);

        // the idea here is to use the "least" diverse queues first
        // so we assign the queue families a score based on their : count , flags
        for (uint32_t i = 0; i < queueFamilyCount; ++i)
        {
            VkQueueFamilyProperties currQueue = queueProps[i];

            float currScore = 0;
            // the less queue are available , the higher the score

            // compute
            if (currQueue.queueFlags & VK_QUEUE_COMPUTE_BIT)
            {
                currScore = (float)1 / currQueue.queueCount;
            }
            computeScore[i] = currScore;

            // gprahics
            currScore = 0;
            if (currQueue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                // the less queue are available , the higher the score
                currScore = (float)1 / currQueue.queueCount;
            }
            graphicsScore[i] = currScore;

            // transfer
            currScore = 0;
            if (currQueue.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                // the less queue are available , the higher the score
                currScore = (float)1 / currQueue.queueCount;
            }
            transferScore[i] = currScore;

            // present
            VkBool32 presentSupported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(currPhysicalDevice, i, *surface, &presentSupported);

            currScore = 0;
            if (presentSupported == VK_TRUE)
            {
                currScore = (float)1 / currQueue.queueCount;
            }

            presentScore[i] = currScore;
        }

        Allocator alloc_empty = STACK_ALLOC(mem_size);
        float *emptyScore = (float *)ALLOC(alloc_empty, mem_size);
        Global::platform.memory.mem_init(emptyScore, mem_size);

        if ((requirements.present) && Global::platform.memory.mem_compare(presentScore, emptyScore, mem_size))
        {
            Global::logger.Error("This device does't support present , skipping ...");
            continue;
        }

        if ((requirements.graphics) && Global::platform.memory.mem_compare(graphicsScore, emptyScore, mem_size))
        {
            Global::logger.Error("This device does't support graphics , skipping ...");
            continue;
        }
        if ((requirements.transfer) && Global::platform.memory.mem_compare(transferScore, emptyScore, mem_size))
        {
            Global::logger.Error("This device does't support transfer , skipping ...");
            continue;
        }
        if ((requirements.compute) && Global::platform.memory.mem_compare(computeScore, emptyScore, mem_size))
        {
            Global::logger.Error("This device does't support compute , skipping ...");
            continue;
        }

        // here we found a GPU that supports all of our requirements
        // so we select the optimal queue index for each requirement
        int presentQueueFamilyIndex = 0;
        int graphicsQueueFamilyIndex = 0;
        int computeQueueFamilyIndex = 0;
        int transferQueueFamilyIndex = 0;

        for (uint32_t i = 1; i < queueFamilyCount; ++i)
        {
            if (presentScore[i] > presentScore[presentQueueFamilyIndex])
            {
                presentQueueFamilyIndex = i;
            }

            if (graphicsScore[i] > graphicsScore[graphicsQueueFamilyIndex])
            {
                graphicsQueueFamilyIndex = i;
            }

            if (computeScore[i] > computeScore[computeQueueFamilyIndex])
            {
                computeQueueFamilyIndex = i;
            }

            if (transferScore[i] > transferScore[transferQueueFamilyIndex])
            {
                transferQueueFamilyIndex = i;
            }
        }

        SwapchainSupportInfo swapchainSupportInfo = {};

        // now we check for swapchain support
        if (!PhysicalDeviceHasSwapchainSupport(currPhysicalDevice, *surface, &swapchainSupportInfo))
        {
            Global::logger.Error("This device doesn't support swapchain , skipping ...");
            continue;
        }

        // if we get here , the physical device is usable
        outDeviceInfo->handle = currPhysicalDevice;
        outDeviceInfo->physicalDeviceFeatures = features;
        outDeviceInfo->physicalDeviceMemoryProperties = memory;
        outDeviceInfo->physicalDeviceProperties = props;
        outDeviceInfo->swapchainSupportInfo = swapchainSupportInfo;
        outDeviceInfo->queues_info.presentQueueFamilyIndex = computeQueueFamilyIndex;
        outDeviceInfo->queues_info.computeQueueFamilyIndex = computeQueueFamilyIndex;
        outDeviceInfo->queues_info.graphicsQueueIndex = graphicsQueueFamilyIndex;
        outDeviceInfo->queues_info.transferQueueIndex = transferQueueFamilyIndex;

        Global::alloc_toolbox.ResetArenaOffset(&checkpoint);
        return true;
    }

    // if we get here , no physical device satisfies the requirements

    Global::alloc_toolbox.ResetArenaOffset(&checkpoint);
    return false;
}

bool CreateGraphicsCommandPools(VulkanContext *context)
{
    VkCommandPoolCreateInfo graphicsPoolCreate = {};
    graphicsPoolCreate.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    graphicsPoolCreate.queueFamilyIndex = context->physical_device_info.queues_info.graphicsQueueIndex;
    graphicsPoolCreate.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    vkCreateCommandPool(context->logical_device_info.handle, &graphicsPoolCreate, context->allocator, &context->physical_device_info.command_pools_info.graphicsCommandPool);
    return true;
}

void HandleWindowResize(WindowResizeEvent evt)
{
    Global::backend_renderer.resize(&Global::backend_renderer, evt.dimensions.x, evt.dimensions.y);
}

bool Startup(BackendRenderer *in_renderer, ApplicationStartup startup)
{
    VulkanContext *ctx = (VulkanContext *)in_renderer->user_data;

    ctx->allocator = nullptr;
    ctx->frame_buffer_size.x = Global::platform.window.width;
    ctx->frame_buffer_size.y = Global::platform.window.height;

    Global::event_system.Listen<WindowResizeEvent>(HandleWindowResize);

    // check for extensions
    uint32_t instance_extCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instance_extCount, nullptr);

    VkExtensionProperties *instance_exts = (VkExtensionProperties *)_alloca(sizeof(VkExtensionProperties) * instance_extCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &instance_extCount, instance_exts);

    Global::logger.Log("Instance extensions available : {} ", instance_extCount);

    for (uint32_t i = 0; i < instance_extCount; ++i)
    {
        Global::logger.Log(instance_exts[i].extensionName);
    }

    // info about our app
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = Defines::engine_name;
    appInfo.pEngineName = Defines::engine_name;

    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
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
    createInfo.enabledExtensionCount = sizeof(INSATNCE_EXTENSION) / sizeof(char *);
    createInfo.ppEnabledExtensionNames = INSATNCE_EXTENSION;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &ctx->vulkan_instance);

    if (result != VK_SUCCESS)
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

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(ctx->vulkan_instance, "vkCreateDebugUtilsMessengerEXT");
    func(ctx->vulkan_instance, &debugInfoCreate, ctx->allocator, &ctx->debug_messenger);

    Global::logger.NewLine(1);

    Global::logger.Log("Vulkan Debugger created");

#endif
    Global::logger.NewLine(1);

    Global::logger.Log("Logical extensions needed :");
    for (const auto &ext : DEVICE_EXTENSIONS)
    {
        Global::logger.Log(ext);
    }

    // surface
    if (!CreateSurface(ctx, &ctx->surface))
    {
        Global::logger.Error("Couldn't create Vulkan Surface ...");
        return false;
    }

    Global::logger.Log("Vulkan surface created");
    Global::logger.NewLine();

    PhysicalDeviceInfo physicalDeviceInfo = {};

    if (!CreatePhysicalDevice(&Global::platform, ctx->vulkan_instance, &ctx->surface, &physicalDeviceInfo))
    {
        Global::logger.Error("Couldn't create Physical device ...");
        return false;
    }

    ctx->physical_device_info = physicalDeviceInfo;

    Global::logger.NewLine();

    Global::logger.Info("> Selected physical device");
    Global::logger.Info("Name : {}", physicalDeviceInfo.physicalDeviceProperties.deviceName);
    Global::logger.Info("Max uniform Buffer range : {} ", physicalDeviceInfo.physicalDeviceProperties.limits.maxUniformBufferRange);

    const char *deviceType = {};
    switch (physicalDeviceInfo.physicalDeviceProperties.deviceType)
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

    Global::logger.Info("GPU Type : {} ", deviceType);

    Global::logger.Info("GPU Driver version : {}.{}.{}",
                        (uint32_t)VK_API_VERSION_MAJOR(physicalDeviceInfo.physicalDeviceProperties.driverVersion),
                        (uint32_t)VK_API_VERSION_MINOR(physicalDeviceInfo.physicalDeviceProperties.driverVersion),
                        (uint32_t)VK_API_VERSION_PATCH(physicalDeviceInfo.physicalDeviceProperties.driverVersion));

    Global::logger.Info("Vulkan API version : {}.{}.{}",
                        VK_API_VERSION_MAJOR(physicalDeviceInfo.physicalDeviceProperties.apiVersion),
                        VK_API_VERSION_MINOR(physicalDeviceInfo.physicalDeviceProperties.apiVersion),
                        VK_API_VERSION_PATCH(physicalDeviceInfo.physicalDeviceProperties.apiVersion));

    Global::logger.NewLine();

    LogicalDeviceRequirements logicalRequirements = {};
    logicalRequirements.samplerAnisotropy = true;

    VkDevice handle = {};
    if (!CreateLogicalDevice(ctx->vulkan_instance, ctx->surface, &ctx->physical_device_info, &logicalRequirements, ctx->allocator, &handle))
    {
        Global::logger.Error("Couldn't create logical device ....");
        return false;
    }

    ctx->logical_device_info.handle = handle;

    Global::logger.Info("Logical device and queue created");

    // create graphics command pool
    if (!CreateGraphicsCommandPools(ctx))
    {
        Global::logger.Error("Couldn't create command pools ....");
        return false;
    }

    Global::logger.Info("Command pools created");

    // create swapshain
    SwapchainCreateDescription swapchainDesc = {};
    swapchainDesc.width = Global::platform.window.width;
    swapchainDesc.height = Global::platform.window.height;
    swapchainDesc.imagesCount = 3;

    if (!SwapchainInfo::Create(ctx, swapchainDesc, VK_NULL_HANDLE, &ctx->swapchain_info))
    {
        Global::logger.Error("Couldn't create swapchain ....");
        return false;
    }

    Global::logger.Info("Swapchain created");

#define UI_PASS true
#define BASIC_PASS false

#if BASIC_PASS == true
    // create basic renderpass
    {
        Rect rect = {};
        rect.x = 0;
        rect.y = 0;
        rect.width = (float)Global::platform.window.width;
        rect.height = (float)Global::platform.window.height;

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

        Renderpass renderpass = {};
        if (!BasicRenderpass::Create(ctx, renderpass_params, &renderpass))
        {
            Global::logger.Error("Couldn't create basic renderpass ....");
            return false;
        }

        DArray<Renderpass>::Add(&ctx->renderPasses, renderpass);

        Global::logger.Info("Basic Renderpass created");
    }
#endif

#if UI_PASS
    // create ui renderpass
    {
        Rect rect = {};
        rect.x = 0;
        rect.y = 0;
        rect.width = (float)Global::platform.window.width;
        rect.height = (float)Global::platform.window.height;

        Color clearColor = {};
        clearColor.r = 0;
        clearColor.g = 0;
        clearColor.b = 0.2f;
        clearColor.a = 1;

        UIRenderpassParams renderpass_params = {};
        renderpass_params.area = rect;
        renderpass_params.clearColor = clearColor;
        renderpass_params.depth = 1;
        renderpass_params.stencil = 0;
        renderpass_params.sync_window_size = true;

        Renderpass renderpass = {};
        if (!UIRenderpass::Create(ctx, renderpass_params, &renderpass))
        {
            Global::logger.Error("Couldn't create ui renderpass ....");
            return false;
        }

        DArray<Renderpass>::Add(&ctx->renderpasses, renderpass);

        Global::logger.Info("UI Renderpass created");
    }
#endif

    // create descriptor set pools
    {
        DescriptorManager::Create(ctx, &ctx->descriptor_manager);
        ctx->descriptor_manager.init_sets_count = 100;
        ctx->descriptor_manager.resize_factor = 2;
    }

    Global::logger.Info("Default shader created");

    // create sampler
    {
        VkSamplerCreateInfo create_sampler = {};
        create_sampler.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        create_sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_sampler.unnormalizedCoordinates = VK_FALSE;
        create_sampler.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_sampler.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_sampler.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        create_sampler.magFilter = VkFilter::VK_FILTER_NEAREST;
        create_sampler.anisotropyEnable = VK_FALSE;
        create_sampler.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;

        vkCreateSampler(ctx->logical_device_info.handle, &create_sampler, ctx->allocator, &ctx->default_sampler);
    }

    Global::logger.Info("Default texture created");

    // staging buffer
    {
        uint32_t staging_alloc_size = 1024 * 1024;

        BufferDescriptor buffer_desc = {};
        VkBufferUsageFlagBits usage = (VkBufferUsageFlagBits)(VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        buffer_desc.memoryPropertyFlags = (VkMemoryPropertyFlagBits)(   VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | 
                                                                        VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        buffer_desc.size = staging_alloc_size;
        buffer_desc.usage = usage;

        if (!Buffer::Create(buffer_desc, true, &ctx->staging_buffer))
        {
            Global::logger.Error("Couldn't create staging buffer ....");
            return false;
        }
    }

    // mesh buffer
    {
        uint32_t mesh_alloc_size = 1024 * 1024;

        BufferDescriptor buffer_desc = {};

        VkBufferUsageFlagBits usage = (VkBufferUsageFlagBits)(VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                              VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                              VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                              VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        buffer_desc.memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        buffer_desc.size = mesh_alloc_size;
        buffer_desc.usage = usage;

        if (!Buffer::Create(buffer_desc, true, &ctx->mesh_buffer))
        {
            Global::logger.Error("Couldn't create vertex buffer ....");
            return false;
        }

        const uint32_t nodes_capacity = 512;
        FreeList::Create(&ctx->mesh_freelist, nodes_capacity, mesh_alloc_size, Global::alloc_toolbox.heap_allocator);
        Global::logger.Info("Vertex buffer created");
    }

    // descriptors buffer
    {
        uint32_t desc_alloc_size = 1024 * 1024;

        BufferDescriptor buffer_desc = {};

        VkBufferUsageFlagBits usage = (VkBufferUsageFlagBits)(VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                              VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                              VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

        buffer_desc.memoryPropertyFlags = (VkMemoryPropertyFlagBits)(VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        buffer_desc.size = desc_alloc_size;
        buffer_desc.usage = usage;

        if (!Buffer::Create(buffer_desc, true, &ctx->descriptors_buffer))
        {
            Global::logger.Error("Couldn't create descriptor buffer ....");
            return false;
        }

        const uint32_t nodes_capacity = 512;
        FreeList::Create(&ctx->descriptors_freelist, nodes_capacity, desc_alloc_size, Global::alloc_toolbox.heap_allocator);
        Global::logger.Info("Descriptor buffer created");
    }

    return true;
}

void Resize(BackendRenderer *in_backend, uint32_t width, uint32_t height)
{
    VulkanContext *ctx = (VulkanContext *)in_backend->user_data;
    ctx->frame_buffer_size.x = width;
    ctx->frame_buffer_size.y = height;
    ctx->frame_buffer_current_counter++;
}

// Recreate the swapchain if needed
// Waits for the last frame's submittion if needed
// Acquires and updates the new image index from the swapchain
// current frame is still the same , it gets updated after "Present"
bool StartFrame(BackendRenderer *in_backend, RendererContext *renderer_ctx)
{
    VulkanContext *ctx = (VulkanContext *)in_backend->user_data;
    LogicalDeviceInfo device = ctx->logical_device_info;

    /*
    if (ctx->recreate_swapchain)
    {
        VkResult result = vkDeviceWaitIdle(device.handle);

        if (result != VkResult::VK_SUCCESS)
        {
            Global::logger.Error("Couldn't wait device idle");
        }

        return false;
    }
    */

    // check if we need to recreate the swapchain
    if (ctx->frame_buffer_current_counter != ctx->frame_buffer_last_counter)
    {
        VkResult result = vkDeviceWaitIdle(device.handle);

        if (result != VkResult::VK_SUCCESS)
        {
            Global::logger.Error("Couldn't wait device idle");
            return false;
        }

        Global::logger.Error("Recreating swapchain");

        SwapchainCreateDescription desc = {};
        desc.width = ctx->frame_buffer_size.x;
        desc.height = ctx->frame_buffer_size.y;
        desc.imagesCount = 3;
        SwapchainInfo::Recreate(ctx, desc, &ctx->swapchain_info);

        for (size_t i = 0; i < ctx->renderpasses.size; ++i)
        {
            Renderpass *curr = &ctx->renderpasses.data[i];
            curr->on_resize(curr);
        }

        ctx->frame_buffer_last_counter = ctx->frame_buffer_current_counter;

        result = vkDeviceWaitIdle(device.handle);
    }

    // first we get the index of the last frame index
    uint32_t last_frame = ctx->current_frame;

    // we wait until the previous frame is done with it's command buffers (or presentation)
    if (!ctx->swapchain_info.cmd_buffer_done_execution_per_frame.data[last_frame].Wait(ctx, UINT64_MAX))
    {
        return false;
    }

    // we ask the swapchain to get us the index of an image that we can render to
    // plug last frame's presentaion semaphore as a "dependency" (in other words , make sure last presentation is donee)
    uint32_t current_image = {};

    if (!ctx->swapchain_info.AcquireNextImageIndex(ctx, UINT32_MAX, ctx->swapchain_info.image_presentation_complete_semaphores.data[last_frame], nullptr, &current_image))
    {
        Global::logger.Error("Couldn't get next image to render to from the swapchain");
        return false;
    }

    ctx->current_image_index = current_image;

    CommandBuffer cmdBuffer = ctx->swapchain_info.graphics_cmd_buffers_per_image.data[current_image];

    cmdBuffer.Reset();
    cmdBuffer.Begin(false, false, false);

    for (size_t i = 0; i < ctx->renderpasses.size; ++i)
    {
        Renderpass *curr = &ctx->renderpasses.data[i];
        curr->begin(curr, &cmdBuffer);
    }

    return true;
}

bool DrawFrame(BackendRenderer *in_backend, RendererContext *renderer_ctx)
{
    VulkanContext *ctx = (VulkanContext *)in_backend->user_data;
    uint32_t current_index = ctx->current_image_index;
    CommandBuffer cmd = ctx->swapchain_info.graphics_cmd_buffers_per_image.data[current_index];

    for (size_t i = 0; i < ctx->renderpasses.size; ++i)
    {
        Renderpass *curr = &ctx->renderpasses.data[i];
        curr->draw(curr, &cmd, renderer_ctx);
    }

    return true;
}

/// Once we reach this function , the image_index represents the current image that's we're working on to send to presentation
bool EndFrame(BackendRenderer *in_backend, RendererContext *rendererContext)
{
    VulkanContext *ctx = (VulkanContext *)in_backend->user_data;
    CommandBuffer cmd = ctx->swapchain_info.graphics_cmd_buffers_per_image.data[ctx->current_image_index];

    for (size_t i = 0; i < ctx->renderpasses.size; ++i)
    {
        Renderpass *curr = &ctx->renderpasses.data[i];
        curr->end(curr, &cmd);
    }

    cmd.End();

    // NOTE : a very important detail to understand here is that we're reusing the Fence used to signal "CommandBuffer finished" to also signal "Frame Present"
    // this might seem confusing at first , but since "Present" always happens after "CommandBuffer finished"
    // we don't really need two different Fences , we can use the Fence to signal "CommandBuffer finished" first
    // then , rest the fence and reuse it to signal "Frame Present"

    // the previous frame might be using the same image as the current frame.
    // if so , we wait it's fence
    Fence *inFlightFence = ctx->swapchain_info.in_flight_fence_per_image.data[ctx->current_image_index];

    if (inFlightFence != VK_NULL_HANDLE)
    {
        if (!inFlightFence->Wait(ctx, UINT64_MAX))
        {
            Global::logger.Error("Couldn't wait previous frame's fence");
        }
    }

    // now the image is free
    // all this does is declare that image[currentImageIndex] is being used by frame[currentFrame] and the fence[currentFrame]
    // we declare that image[0] for example, is now associated with the command using fence[2] for example
    // (notice how the frame_index doesn't necessarily have to match the image_index)
    // once the fence is signaled , we know that the image is free to be reused

    // so , here  we take the the address of the Fence that we just waited for
    Fence *submit_fence = &ctx->swapchain_info.cmd_buffer_done_execution_per_frame.data[ctx->current_frame];

    // reset it
    submit_fence->Reset(ctx);

    // then reuse it as "in flight" fence
    ctx->swapchain_info.in_flight_fence_per_image.data[ctx->current_image_index] = submit_fence;

    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.commandBufferCount = 1;
    info.pCommandBuffers = &cmd.handle;

    // signaled when the queue is complete
    info.signalSemaphoreCount = 1;
    info.pSignalSemaphores = &ctx->swapchain_info.finished_rendering_semaphores.data[ctx->current_frame];

    // wait semaphore to make sure that the operation cannot begin until the image is available
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &ctx->swapchain_info.image_presentation_complete_semaphores.data[ctx->current_frame];

    VkPipelineStageFlags pipelineFlags[1] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    info.pWaitDstStageMask = &pipelineFlags[0];

    // this is an async (non-blocking call) that returns immidiately
    // to know if the sumbission is done on the GPU side , we will need to check the Fence passed
    // from the docs : fence is an optional handle to a fence to be signaled once all submitted command buffers have completed execution
    vkQueueSubmit(ctx->physical_device_info.queues_info.graphics_queue, 1, &info, submit_fence->handle);

    cmd.UpdateSubmitted();

    // note : accordring to the spec
    // presentation requests sent in a particular queue are always performed in order
    // that way , we can be sure that present order is always 0,1,2,0,1,2,0,1,2....
    ctx->swapchain_info.Present(
        ctx,
        // wait the finish rendering semaphore
        // this will make sure that we send the image for presentation AFTER all the rendering commands are done drawing to it
        ctx->swapchain_info.finished_rendering_semaphores.data[ctx->current_frame],
        &ctx->current_image_index);

    // increments and loop frame count
    ctx->current_frame = (ctx->current_frame + 1) % ctx->swapchain_info.images_count;

    return true;
}

bool Destroy(BackendRenderer *in_backend)
{
    VulkanContext *ctx = (VulkanContext *)in_backend->user_data;
    Global::event_system.Unlisten<WindowResizeEvent>(HandleWindowResize);

    vkDeviceWaitIdle(ctx->logical_device_info.handle);

    Buffer::Destroy(&ctx->mesh_buffer);
    FreeList::Destroy(&ctx->mesh_freelist);

    Buffer::Destroy(&ctx->descriptors_buffer);
    FreeList::Destroy(&ctx->descriptors_freelist);

    Buffer::Destroy(&ctx->staging_buffer);
    
    DescriptorManager::Destroy(&ctx->descriptor_manager);

    for (size_t i = 0; i < ctx->renderpasses.size; ++i)
    {
        Renderpass *curr = &ctx->renderpasses.data[i];
        curr->on_destroy(curr);

        vkDestroyRenderPass(ctx->logical_device_info.handle, curr->handle, ctx->allocator);
    }

    DArray<Renderpass>::Destroy(&ctx->renderpasses);

    SwapchainInfo::Destroy(ctx, &ctx->swapchain_info);

    vkDestroySampler(ctx->logical_device_info.handle, ctx->default_sampler, ctx->allocator);
    vkDestroySurfaceKHR(ctx->vulkan_instance, ctx->surface, ctx->allocator);
    vkDestroyCommandPool(ctx->logical_device_info.handle, ctx->physical_device_info.command_pools_info.graphicsCommandPool, ctx->allocator);
    vkDestroyDevice(ctx->logical_device_info.handle, ctx->allocator);

    return true;
}

inline void WaitIdle(BackendRenderer *in_renderer)
{
    VulkanContext *ctx = (VulkanContext *)in_renderer->user_data;
    vkDeviceWaitIdle(ctx->logical_device_info.handle);
}

void VulkanBackendRenderer::Create(BackendRenderer *out_renderer)
{
    VulkanContext *ctx = Global::alloc_toolbox.HeapAlloc<VulkanContext>();
    DArray<Renderpass>::Create(3, &ctx->renderpasses, Global::alloc_toolbox.heap_allocator);

    out_renderer->user_data = ctx;
    out_renderer->startup = Startup;
    out_renderer->resize = Resize;
    out_renderer->start_frame = StartFrame;
    out_renderer->end_frame = EndFrame;
    out_renderer->draw_frame = DrawFrame;
    out_renderer->destroy = Destroy;
    out_renderer->wait_idle = WaitIdle;
}

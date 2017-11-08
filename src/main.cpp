/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Vulkan test application.
 * ---------------------------------------------------------------- */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_device.h"
#include "vk_instance.h"
#include "vk_mesh.h"
#include "vk_physical_device.h"
#include "vk_queue.h"
#include "vk_shader_stage.h"

/* ---------------------------------------------------------------- *
   Globals.
 * ---------------------------------------------------------------- */

// Window
static const char* WINDOW_NAME = "Vulkan test";
static const int WINDOW_WIDTH  = 720;
static const int WINDOW_HEIGHT = 576;

// Surface
static const VkFormat SURFACE_FORMAT             = VK_FORMAT_R8G8B8A8_UNORM;
static const VkColorSpaceKHR SURFACE_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Presentation
static const VkPresentModeKHR PRESENT_MODE = VK_PRESENT_MODE_FIFO_KHR; // v-sync

// Swap chain
static const int SWAP_CHAIN_IMAGE_COUNT = 2;

#if 0
/* ---------------------------------------------------------------- *
   Validation layer debug callback.
 * ---------------------------------------------------------------- */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT /*flags*/,
    VkDebugReportObjectTypeEXT /*objType*/,
    uint64_t /*obj*/,
    size_t /*location*/,
    int32_t /*code*/,
    const char* /*layerPrefix*/,
    const char* msg,
    void* /*userData*/) {

    std::cerr << __FUNCTION__
              << ": Vulkan validation layer: "
              << msg
              << std::endl;
    return VK_FALSE;
}

/* ---------------------------------------------------------------- *
   Reads a SPIR-V binary shader file form the path.
 * ---------------------------------------------------------------- */
std::vector<char> readShaderSourceFile(const std::string& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        std::cerr << __FUNCTION__
                  << "failed to open shader file "
                  << path.c_str()
                  << std::endl;

    size_t fileSize = (size_t) file.tellg();
    if (fileSize == 0)
        std::cerr << __FUNCTION__
                  << "shader source is zero-sized"
                  << path.c_str()
                  << std::endl;

    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}
#endif

int main()
{
    /* ------------------------------------------------------------ *
       GLFW window.
     * ------------------------------------------------------------ */

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME,
        nullptr, nullptr);
    if (!window)
    {
        std::cerr << __FUNCTION__
                  << ": Failed to create Vulkan GLFW window"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Instance
     * ------------------------------------------------------------ */

    using namespace kuu::vk;

    // Get the extensions that GLFW window requires. On Windows
    // these are VK_KHR_surface and VK_KHR_win32_surface.
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(
        &glfwExtensionCount);
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        const std::string glfwExtension = glfwExtensions[i];
        if (!Instance::isExtensionSupported(glfwExtension))
            throw std::runtime_error(
                __FUNCTION__ +
                    std::string(": Vulkan implementation does not support ") +
                    glfwExtension + " extension.");
    }

    std::vector<std::string> instanceLayers;
    std::vector<std::string> instanceExtensions;
    for (uint32_t e = 0; e < glfwExtensionCount; ++e)
        instanceExtensions.push_back(std::string(glfwExtensions[e]));

    // Check if the validation layer is available
    bool enableValidationLayer = Instance::isLayerSupported("VK_LAYER_LUNARG_standard_validation");
    if (enableValidationLayer)
    {
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    Instance inst(WINDOW_NAME, "kuuEngine",
                  instanceExtensions, instanceLayers);
    VkInstance instance = inst.handle();
    inst.setValidationLeyerEnabled();

#if 0
    /* ------------------------------------------------------------ *
       Instance extensions.
     * ------------------------------------------------------------ */

    // Get the extensions count.
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           nullptr);

    // Get the extensions.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           extensions.data());

    // Get the extensions that GLFW window requires. On Windows
    // these are VK_KHR_surface and VK_KHR_win32_surface.
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(
        &glfwExtensionCount);

    // Check that the required extensions exits.
    for (uint32_t e = 0; e < glfwExtensionCount; ++e)
    {
        const char* glfwExtension = glfwExtensions[e];
        const auto it = std::find_if(
            extensions.begin(),
            extensions.end(),
            [glfwExtension](const VkExtensionProperties& ex)
        {
            return std::string(ex.extensionName) ==
                   std::string(glfwExtension);
        });

        if (it == extensions.end())
        {
            std::cerr << __FUNCTION__
                      << ": Vulkan implementation does not support "
                         "extension"
                      << glfwExtension
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    std::vector<const char*> instanceExtensions;
    for (uint32_t e = 0; e < glfwExtensionCount; ++e)
        instanceExtensions.push_back(glfwExtensions[e]);

    /* ------------------------------------------------------------ *
       Instance layers.
     * ------------------------------------------------------------ */

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount,
                                       layers.data());

    // Check if the validation layer is available
    std::vector<const char*> validationLayer;
    validationLayer.push_back("VK_LAYER_LUNARG_standard_validation");
    bool enableValidationLayer = false;
    for (const VkLayerProperties& layer : layers)
        if (std::string(layer.layerName) ==
            std::string(validationLayer[0]))
        {
            enableValidationLayer = true;
        }

    if (enableValidationLayer)
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    /* ------------------------------------------------------------ *
       Instance.
     * ------------------------------------------------------------ */

    // Let the implementation know that this application does not
    // use any famous 3rd party engine.
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = WINDOW_NAME;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "kuuEngine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    // Create the instance with extensions that GLFW requires.
    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledExtensionCount   = uint32_t(instanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();
    if (enableValidationLayer)
    {
        instanceInfo.enabledLayerCount   = 1;
        instanceInfo.ppEnabledLayerNames = validationLayer.data();
    }
    else
        instanceInfo.enabledLayerCount       = 0;

    VkInstance instance;
    VkResult result = vkCreateInstance(&instanceInfo,
                                       nullptr,
                                       &instance);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create Vulkan instance"
                  << std::endl;

        return EXIT_FAILURE;
    }
#endif
    VkResult result;

    /* ------------------------------------------------------------ *
       Debug callback.
     * ------------------------------------------------------------ */
#if 0
    VkDebugReportCallbackEXT callback;
    if (enableValidationLayer)
    {
        VkDebugReportCallbackCreateInfoEXT debugInfo = {};
        debugInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugInfo.flags        = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debugInfo.pfnCallback = debugCallback;

        // Validation is an extension, get the function adress.
        auto func = (PFN_vkCreateDebugReportCallbackEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkCreateDebugReportCallbackEXT");
        if (func)
            result = func(instance, &debugInfo, nullptr, &callback);
        else
            result = VK_ERROR_INITIALIZATION_FAILED;
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan debug "
                      << "report callback"
                      << std::endl;

            return EXIT_FAILURE;
        }
    }
#endif

    /* ------------------------------------------------------------ *
       Surface.
     * ------------------------------------------------------------ */

    // Create the GLFW vulkan surface
    VkSurfaceKHR surface;
    result = glfwCreateWindowSurface(instance,
                                     window,
                                     nullptr,
                                     &surface);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan surface"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Physical device.
     * ------------------------------------------------------------ */

    std::vector<std::string> physicalDeviceExtensionNames;
    physicalDeviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    std::vector<PhysicalDevice> physicalDevices =
        inst.physicalDevices(surface);

    std::vector<PhysicalDevice> validPhysicalDevices;
    for (PhysicalDevice device : physicalDevices)
    {
        device.dump();

        if (!device.isExtensionSupported(physicalDeviceExtensionNames))
            continue;
        if (!device.isSurfaceSupported(SURFACE_FORMAT, SURFACE_COLOR_SPACE))
            continue;
        if (!device.isPresentModeSupported(PRESENT_MODE))
            continue;
        if (!device.isImageExtentSupported(glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT)))
            continue;
        if (!device.isSwapChainImageCountSupported(SWAP_CHAIN_IMAGE_COUNT))
            continue;

        validPhysicalDevices.push_back(device);
    }

    if (validPhysicalDevices.size() == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find valid physical device "
                  << std::endl;

        return EXIT_FAILURE;
    }

#if 0
    // Get the device count.
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance,
                               &physicalDeviceCount,
                               nullptr);

    if (physicalDeviceCount == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find devices with Vulkan support"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Get the devices.
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance,
                               &physicalDeviceCount,
                               physicalDevices.data());

    // Device is valid if it has VK_KHR_swapchain extension and
    // supports the global surface format and present mode.
    std::vector<const char*> physicalDeviceExtensionNames;
    physicalDeviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Search the valid physical devices.
    std::vector<VkPhysicalDevice> validPhysicalDevices;
    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
    {
#if 0
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
#endif
        // Get the surface format count.
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                             surface,
                                             &formatCount,
                                             nullptr);
        if (formatCount == 0)
            continue;

        // Get the surface formats.
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
                                             surface,
                                             &formatCount,
                                             formats.data());

        // Check that the device supports the surface mode.
        const auto it = std::find_if(
            formats.begin(),
            formats.end(),
            [](const VkSurfaceFormatKHR& fmt)
        {
            return fmt.format     == SURFACE_FORMAT &&
                   fmt.colorSpace == SURFACE_COLOR_SPACE;
        });
        if (it == formats.end())
            continue;

        // Get the present modes count.
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            nullptr);

        if (presentModeCount == 0)
            continue;

        // Get the present modes.
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice,
                                                  surface,
                                                  &presentModeCount,
                                                  presentModes.data());

        // Check that the device supports the present mode.
        const auto it2 = std::find_if(
            presentModes.begin(),
            presentModes.end(),
            [](const VkPresentModeKHR& mode)
        { return mode == PRESENT_MODE; });
        if (it2 == presentModes.end())
            continue;

        // Get the surface capabilites.
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,
                                                  surface,
                                                  &capabilities);

        // Check the window extent is supported.
        if (WINDOW_WIDTH  < capabilities.minImageExtent.width  ||
            WINDOW_WIDTH  > capabilities.maxImageExtent.width  ||
            WINDOW_HEIGHT < capabilities.minImageExtent.height ||
            WINDOW_HEIGHT > capabilities.maxImageExtent.height)
        {
            continue;
        }

        // Check the image count is supported.
        if (SWAP_CHAIN_IMAGE_COUNT < capabilities.minImageCount ||
            SWAP_CHAIN_IMAGE_COUNT > capabilities.maxImageCount)
        {
            continue;
        }

        // Get the extension count
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice,
                                             nullptr,
                                             &extensionCount,
                                             nullptr);

        // Get the extensions
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice,
                                             nullptr,
                                             &extensionCount,
                                             extensions.data());

        // Check that the device supports all the extensions.
        int found = 0;
        for (const VkExtensionProperties& ex : extensions)
        {
            for (const char* name : physicalDeviceExtensionNames)
                if (std::string(ex.extensionName) == std::string(name))
                    found++;
        }

        if (found != physicalDeviceExtensionNames.size())
            continue;

        // OK, device is valid.
        validPhysicalDevices.push_back(physicalDevice);
    }

    if (validPhysicalDevices.size() == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find valid physical device "
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Auto-select the first device.
    VkPhysicalDevice physicalDevice = validPhysicalDevices.front();
#endif

    // Auto-select the first device.
    PhysicalDevice phyDev = validPhysicalDevices[0];
    VkPhysicalDevice physicalDevice = phyDev.handle();

    Queue graphicsQueue(physicalDevice,
                        surface,
                        Queue::Type::Graphics);
    graphicsQueue.create(1, 1.0f);

    Queue presentQueue(physicalDevice,
                       surface,
                       Queue::Type::Presentation);
    presentQueue.create(1, 1.0f);

    std::vector<Queue> queues;
    queues.push_back(graphicsQueue);
    queues.push_back(presentQueue);

    Device dev(physicalDevice);
    dev.create(queues, physicalDeviceExtensionNames);

    VkDevice device = dev.handle();

#if 0

    /* ------------------------------------------------------------ *
       Queue families (graphics and present)
     * ------------------------------------------------------------ */

    // Get the queue family count.
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                             &queueFamilyCount,
                                             nullptr);

    if (queueFamilyCount == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find Vulkan queue families"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Get the queue families.
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                             &queueFamilyCount,
                                             queueFamilies.data());

    // Find the graphics and present queue family indices.
    uint32_t graphicsQueueFamilyIndex = -1;
    uint32_t presentQueueFamilyIndex  = -1;
    for (int i = 0; i < queueFamilies.size(); ++i)
    {
        const VkQueueFamilyProperties& queueFamily = queueFamilies[i];
        if (queueFamily.queueCount <= 0)
            continue;

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsQueueFamilyIndex = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                                             i,
                                             surface,
                                             &presentSupport);
        if (presentSupport)
            presentQueueFamilyIndex = i;
    }

    if (graphicsQueueFamilyIndex == -1)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find Vulkan graphics queue family"
                  << std::endl;

        return EXIT_FAILURE;
    }

    if (presentQueueFamilyIndex == -1)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find Vulkan present queue family"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Logical device.
     * ------------------------------------------------------------ */

    // Create the queue create infos for graphics and present queues.
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueInfos(2);

    queueInfos[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[0].queueFamilyIndex = graphicsQueueFamilyIndex;
    queueInfos[0].queueCount       = 1;
    queueInfos[0].pQueuePriorities = &queuePriority;

    queueInfos[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfos[1].queueFamilyIndex = presentQueueFamilyIndex;
    queueInfos[1].queueCount       = 1;
    queueInfos[1].pQueuePriorities = &queuePriority;

    std::vector<const char*> physicalDeviceExtensionNamesStr;
    for (auto& extension : physicalDeviceExtensionNames)
        physicalDeviceExtensionNamesStr.push_back(extension.c_str());

    // Logical device with graphics and present queues.
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pQueueCreateInfos       = queueInfos.data();
    deviceInfo.queueCreateInfoCount    = 2;
    deviceInfo.pEnabledFeatures        = &deviceFeatures;
    deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(physicalDeviceExtensionNamesStr.size());
    deviceInfo.ppEnabledExtensionNames = physicalDeviceExtensionNamesStr.data();
    deviceInfo.enabledLayerCount       = 0;

    // Create the logical device.
    VkDevice device;
    result = vkCreateDevice(physicalDevice,
                            &deviceInfo,
                            nullptr,
                            &device);
    if (result != VK_SUCCESS)
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan logical device"
                  << std::endl;
#endif

    /* ------------------------------------------------------------ *
       Swap chain
     * ------------------------------------------------------------ */

    std::vector<uint32_t> queueFamilyIndices;
    for (const Queue& q : queues)
        queueFamilyIndices.push_back(q.familyIndex());

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface          = surface;
    createInfo.minImageCount    = SWAP_CHAIN_IMAGE_COUNT;
    createInfo.imageFormat      = SURFACE_FORMAT;
    createInfo.imageColorSpace  = SURFACE_COLOR_SPACE;
    createInfo.imageExtent      = VkExtent2D { WINDOW_WIDTH, WINDOW_HEIGHT };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode      = PRESENT_MODE;
    createInfo.clipped          = VK_TRUE;
    createInfo.oldSwapchain     = VK_NULL_HANDLE;
    createInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    if (graphicsQueue.familyIndex() != presentQueue.familyIndex())
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = uint32_t(queueFamilyIndices.size());
        createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSwapchainKHR swapChain;
    result = vkCreateSwapchainKHR(device,
                                  &createInfo,
                                  nullptr,
                                  &swapChain);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan swap chain"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Get the swap chain image count.
    uint32_t imageCount = 0;
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);

    // Get the swap chain images.
    std::vector<VkImage> swapChainImages(imageCount);
    vkGetSwapchainImagesKHR(device,
                            swapChain,
                            &imageCount,
                            swapChainImages.data());

    // Create swap chain image views.
    std::vector<VkImageView> swapChainImageViews(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image    = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format   = SURFACE_FORMAT;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device,
                                   &createInfo,
                                   nullptr,
                                   &swapChainImageViews[i]);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan image view"
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    /* ------------------------------------------------------------ *
       Shader
     * ------------------------------------------------------------ */

    using namespace kuu::vk;
    ShaderStage shader(device);
    shader.create("shaders/test.vert.spv",
                  "shaders/test.frag.spv");

#if 0
    /* ------------------------------------------------------------ *
       Vertex shader
     * ------------------------------------------------------------ */

    const std::vector<char> vshSrc =
        readShaderSourceFile("shaders/test.vert.spv");

    VkShaderModuleCreateInfo vshInfo = {};
    vshInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vshInfo.codeSize = vshSrc.size();
    vshInfo.pCode    = reinterpret_cast<const uint32_t*>(vshSrc.data());

    VkShaderModule vshModule;
    result = vkCreateShaderModule(device, &vshInfo, nullptr, &vshModule);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan vertex shader module"
                  << std::endl;

        return EXIT_FAILURE;
    }

    VkPipelineShaderStageCreateInfo vshStageInfo = {};
    vshStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vshStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
    vshStageInfo.module = vshModule;
    vshStageInfo.pName  = "main";

    /* ------------------------------------------------------------ *
       Fragment shader
     * ------------------------------------------------------------ */

    const std::vector<char> fshSrc =
        readShaderSourceFile("shaders/test.frag.spv");

    VkShaderModuleCreateInfo fshInfo = {};
    fshInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fshInfo.codeSize = fshSrc.size();
    fshInfo.pCode    = reinterpret_cast<const uint32_t*>(fshSrc.data());

    VkShaderModule fshModule;
    result = vkCreateShaderModule(device, &fshInfo, nullptr, &fshModule);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan vertex shader module"
                  << std::endl;

        return EXIT_FAILURE;
    }

    VkPipelineShaderStageCreateInfo fshStageInfo = {};
    fshStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fshStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    fshStageInfo.module = fshModule;
    fshStageInfo.pName  = "main";
#endif

    /* ------------------------------------------------------------ *
       Rasterizer state
     * ------------------------------------------------------------ */

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable        = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth               = 1.0f;
    rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable         = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp          = 0.0f;
    rasterizer.depthBiasSlopeFactor    = 0.0f;

    /* ------------------------------------------------------------ *
       Multisampling stage
     * ------------------------------------------------------------ */

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 1.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;

    /* ------------------------------------------------------------ *
       Blending state
     * ------------------------------------------------------------ */

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable         = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable     = VK_FALSE;
    colorBlending.logicOp           = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount   = 1;
    colorBlending.pAttachments      = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    /* ------------------------------------------------------------ *
       Vulkan pipeline layout
     * ------------------------------------------------------------ */

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = 0;
    pipelineLayoutInfo.pSetLayouts            = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges    = 0;

    VkPipelineLayout pipelineLayout;
    result = vkCreatePipelineLayout(device,
                                    &pipelineLayoutInfo,
                                    nullptr,
                                    &pipelineLayout);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan pipeline layout"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Viewport state.
     * ------------------------------------------------------------ */

    // Viewport
    VkViewport viewport = {};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = WINDOW_WIDTH;
    viewport.height   = WINDOW_HEIGHT;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor
    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = VkExtent2D { WINDOW_WIDTH, WINDOW_HEIGHT };

    // Viewport state.
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    /* ------------------------------------------------------------ *
       Render pass

        * framebuffer attachments
        * framebuffer content handling

     * ------------------------------------------------------------ */

    // Create the description of the colorbuffer attachment. The
    // colorbuffer is an image in the swap chain.
    //      * no multisampling
    //      * clear the content before rendering
    //      * keep the contetn after rendering
    //      * do not care stencil operations
    //      * do not create what is the pixel layout before rendering
    //        as it is going to be cleared
    //      * set pixel layout to be presentation format after
    //        rendering as its going to be display on the screen
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format         = SURFACE_FORMAT;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Attachment descriptions
    const VkAttachmentDescription attachmentDescriptions[] =
    { colorAttachment };

    // Graphics attachment reference for subpass which is the color
    // attachment above. Use the optimal layout for color attachment
    // as thats what it is.
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Subpass that renders into colorbuffer attachment.
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    // Add a subpass dependecy to prevent the image layout transition
    // being run too early. Make render pass to wait by waiting for
    // the color attachment output stage.
    VkSubpassDependency dependency = {};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Fill the render pass info.
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = attachmentDescriptions;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    // Create the render pass.
    VkRenderPass renderPass;
    result = vkCreateRenderPass(device, &renderPassInfo,
                                nullptr, &renderPass);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan render pass"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Mesh
     * ------------------------------------------------------------ */

    const std::vector<Vertex> vertices =
    {
        {  {0.0f, -0.5f}, { 1.0f, 0.0f, 0.0f } },
        {  {0.5f,  0.5f}, { 0.0f, 1.0f, 0.0f } },
        { {-0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f } }
    };

    Mesh mesh(device, physicalDevice, 0);
    mesh.create(vertices);

    /* ------------------------------------------------------------ *
       Graphics pipeline
     * ------------------------------------------------------------ */

    // Shader stages
#if 0
    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        vshStageInfo,
        fshStageInfo
    };
#endif

    auto shaderStages = shader.shaderStages();

    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    bindingDescriptions.push_back(mesh.bindingDescription());
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    for (auto desc : mesh.attributeDescriptions())
        attributeDescriptions.push_back(desc);

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount   = uint32_t(bindingDescriptions.size());
    vertexInput.pVertexBindingDescriptions      = bindingDescriptions.data();
    vertexInput.vertexAttributeDescriptionCount = uint32_t(attributeDescriptions.size());
    vertexInput.pVertexAttributeDescriptions    = attributeDescriptions.data();

    // Input assemply state
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = uint32_t(shaderStages.size());
    pipelineInfo.pStages             = shaderStages.data();
    pipelineInfo.pVertexInputState   = &vertexInput;
    pipelineInfo.pViewportState      = &viewportState;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState   = &multisampling;
    pipelineInfo.pDepthStencilState  = nullptr;
    pipelineInfo.pColorBlendState    = &colorBlending;
    pipelineInfo.pDynamicState       = nullptr;
    pipelineInfo.layout              = pipelineLayout;
    pipelineInfo.renderPass          = renderPass;
    pipelineInfo.subpass             = 0;
    pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex   = -1;

    VkPipeline graphicsPipeline;
    result = vkCreateGraphicsPipelines(device,
                                       VK_NULL_HANDLE,
                                       1,
                                       &pipelineInfo,
                                       nullptr,
                                       &graphicsPipeline);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan graphics pipelines"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Framebuffers of swap chain images for render pass.
     * ------------------------------------------------------------ */

    // Each swap chain image view has its own framebuffer.
    std::vector<VkFramebuffer> swapChainFramebuffers;
    swapChainFramebuffers.resize(swapChainImageViews.size());

    // Create the framebuffers
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        const VkImageView attachments[] = { swapChainImageViews[i] };

        // Create the framebuffer info.
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = WINDOW_WIDTH;
        framebufferInfo.height          = WINDOW_HEIGHT;
        framebufferInfo.layers          = 1;

        // Create the framebuffer.
        result = vkCreateFramebuffer(device, &framebufferInfo,
                                     nullptr,
                                     &swapChainFramebuffers[i]);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan framebuffer"
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    /* ------------------------------------------------------------ *
       Command buffer.
     * ------------------------------------------------------------ */

    // Create the command pool info for graphics queue. Commands
    // are recorded only once and the executed multiple times on
    // the main loop (flags = 0).
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsQueue.familyIndex();
    poolInfo.flags            = 0;

    // Create command pool.
    VkCommandPool commandPool;
    result = vkCreateCommandPool(device, &poolInfo,
                                 nullptr, &commandPool);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan command pool"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Each image in the swap chain needs to have its own
    // command buffer.
    std::vector<VkCommandBuffer> commandBuffers;
    commandBuffers.resize(swapChainFramebuffers.size());

    // Create the command buffer allocation information.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool        = commandPool;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = uint32_t(commandBuffers.size());

    // Allocate the command buffers.
    result = vkAllocateCommandBuffers(device, &allocInfo,
                                      commandBuffers.data());
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to allocate command buffers"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Record same commands for each command buffer.
    for (size_t i = 0; i < commandBuffers.size(); i++)
    {
        // Create the begin info where the command buffer can be
        // resubmitted while is pending for execution.
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        // Begin recording commands
        vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

        // Clear color for colorbuffer attachment
        VkClearValue clearColor = { 0.2f, 0.2f, 0.2f, 1.0f };

        // Create the render pass begin info. Set the framebuffer
        // and the size of it + clear color.
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = renderPass;
        renderPassInfo.framebuffer       = swapChainFramebuffers[i];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = VkExtent2D { WINDOW_WIDTH, WINDOW_HEIGHT };
        renderPassInfo.clearValueCount   = 1;
        renderPassInfo.pClearValues      = &clearColor;

        // Begin the render pass.
        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        // Bind the graphics pipeline
        vkCmdBindPipeline(commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphicsPipeline);

        mesh.draw(commandBuffers[i]);

        // End the render pass.
        vkCmdEndRenderPass(commandBuffers[i]);

        // End recording commands.
        result = vkEndCommandBuffer(commandBuffers[i]);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to end command buffer"
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    /* ------------------------------------------------------------ *
       Render / Presentation sync
     * ------------------------------------------------------------ */

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Create semaphore for rendering to know when an image in
    // swap chain is available
    VkSemaphore imageAvailableSemaphore;
    result = vkCreateSemaphore(device,
                               &semaphoreInfo,
                               nullptr,
                               &imageAvailableSemaphore);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create semaphore for image availability"
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Create semaphore for presentation to know when an image in
    // swap chain is rendered.
    VkSemaphore renderFinishedSemaphore;
    result = vkCreateSemaphore(device,
                               &semaphoreInfo,
                               nullptr,
                               &renderFinishedSemaphore);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create semaphore for render finished"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Draw into framebuffer attachment.
     * ------------------------------------------------------------ */

    // Acquire an image from the swap chain. Use the semaphore to
    // detect when the present queue has finished using the image.
    // The image is a color attachment to framebuffer.
    const uint64_t timeout = std::numeric_limits<uint64_t>::max(); // this disables timeout...?
    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(device, swapChain, timeout,
                          imageAvailableSemaphore,
                          VK_NULL_HANDLE, &imageIndex);

    // Semaphores to wait until the queue submit is executed
    // and which stages of pipeline the semaphore is  waited.
    const VkSemaphore waitSemaphores[]      = { imageAvailableSemaphore };
    const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    // Semaphore to signal when the command buffer has finished
    // execution.
    VkSemaphore signalSemaphores[]  = { renderFinishedSemaphore };

    // Create the graphics queue submit info.
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;



    // Get the graphics queue
//    VkQueue graphicsQueue;
//    vkGetDeviceQueue(device,
//                     graphicsQueue.familyIndex(),
//                     0,
//                     &graphicsQueue);

    // Submit the command buffer into queue
    result = vkQueueSubmit(graphicsQueue.handle(),
                           1,
                           &submitInfo,
                           VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to submit into graphics queue"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Present the framebuffer from swap chain into screen surface.
     * ------------------------------------------------------------ */

    // Get the present queue handle from the logical device.
//    VkQueue presentQueue;
//    vkGetDeviceQueue(device, presentQueue.familyIndex(),
//                     0, &presentQueue);

    // Create the present info which tells:
    //   * semaphore to wait until the rendering result image can
    //     be presented to screen surface
    //   * swap chain and swap chain image to present to screen.
    //   * (other options are needed only when using multiple swap
    //      chains)
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &swapChain;
    presentInfo.pImageIndices      = &imageIndex;
    presentInfo.pResults           = nullptr;

    // Present the rendering result into screen surface.
    result = vkQueuePresentKHR(presentQueue.handle(), &presentInfo);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to present the rendering results "
                  << "to screen surface."
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Wait until the present queue has finished.
    result = vkQueueWaitIdle(presentQueue.handle());
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to wait until the rendering image is "
                  << "presented onto screen. "
                  << "to screen surface."
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Start the "event" loop.
     * ------------------------------------------------------------ */

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vkDeviceWaitIdle(device);

    /* ------------------------------------------------------------ *
       Clean up
     * ------------------------------------------------------------ */

    glfwDestroyWindow(window);
    glfwTerminate();

    vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);

    for (uint32_t i = 0; i < swapChainFramebuffers.size(); i++)
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    for (uint32_t i = 0; i < swapChainImageViews.size(); i++)
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);

    shader.destroy();
    mesh.destroy();

    vkDestroyCommandPool(device, commandPool, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
//    vkDestroyShaderModule(device, fshModule, nullptr);
//    vkDestroyShaderModule(device, vshModule, nullptr);
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
//    vkDestroyDevice(device, nullptr);

//    if (enableValidationLayer)
//    {
//        auto func = (PFN_vkDestroyDebugReportCallbackEXT)
//            vkGetInstanceProcAddr(instance,
//                                  "vkDestroyDebugReportCallbackEXT");
//        if (func)
//            func(instance, callback, nullptr);
//    }

//    vkDestroyInstance(instance, nullptr);

    return EXIT_SUCCESS;
}

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
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

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

int main()
{
    /* ------------------------------------------------------------ *
       Vulkan GLFW window.
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
       Vulkan instance extensions.
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
       Vulkan instance layers.
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
       Vulkan instance.
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

    /* ------------------------------------------------------------ *
       Vulkan debug callback.
     * ------------------------------------------------------------ */

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

    /* ------------------------------------------------------------ *
       Vulkan surface.
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
       Vulkan physical device.
     * ------------------------------------------------------------ */

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

    /* ------------------------------------------------------------ *
       Vulkan queue families (graphics and present)
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
       Vulkan logical device.
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

    // Logical device with graphics and present queues.
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo deviceInfo = {};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pQueueCreateInfos       = queueInfos.data();
    deviceInfo.queueCreateInfoCount    = 2;
    deviceInfo.pEnabledFeatures        = &deviceFeatures;
    deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(physicalDeviceExtensionNames.size());
    deviceInfo.ppEnabledExtensionNames = physicalDeviceExtensionNames.data();
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

    /* ------------------------------------------------------------ *
       Vulkan swap chain
     * ------------------------------------------------------------ */

    const uint32_t queueFamilyIndices[] =
    {
        graphicsQueueFamilyIndex,
        presentQueueFamilyIndex
    };

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

    if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
    {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
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
       Vulkan vertex shader
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
       Vulkan fragment shader
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

    /* ------------------------------------------------------------ *
       Vulkan rasterizer
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
       Vulkan multisampling
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
       Vulkan blending
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
       Vulkan viewport state.
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
    scissor.offset = {0, 0};
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
     * ------------------------------------------------------------ */

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format         = SURFACE_FORMAT;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments    = &colorAttachment;
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;

    VkRenderPass renderPass;
    result = vkCreateRenderPass(device,
                                &renderPassInfo,
                                nullptr,
                                &renderPass);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan render pass"
                  << std::endl;

        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------ *
       Vulkan graphics pipeline
     * ------------------------------------------------------------ */

    // Shader stages
    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        vshStageInfo,
        fshStageInfo
    };

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInput = {};
    vertexInput.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.vertexBindingDescriptionCount   = 0;
    vertexInput.pVertexBindingDescriptions      = nullptr;
    vertexInput.vertexAttributeDescriptionCount = 0;
    vertexInput.pVertexAttributeDescriptions    = nullptr;

    // Input assemply state
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Graphics pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount          = 2;
    pipelineInfo.pStages             = shaderStages;
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
       Vulkan framebuffers
     * ------------------------------------------------------------ */

    std::vector<VkFramebuffer> swapChainFramebuffers(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments    = attachments;
        framebufferInfo.width           = WINDOW_WIDTH;
        framebufferInfo.height          = WINDOW_HEIGHT;
        framebufferInfo.layers          = 1;

        result = vkCreateFramebuffer(device,
                                     &framebufferInfo,
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
       Vulkan logical device queues
     * ------------------------------------------------------------ */

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device,
                     graphicsQueueFamilyIndex,
                     0,
                     &graphicsQueue);

    VkQueue presentQueue;
    vkGetDeviceQueue(device,
                     presentQueueFamilyIndex,
                     0,
                     &presentQueue);

    /* ------------------------------------------------------------ *
       Start the "event" loop.
     * ------------------------------------------------------------ */

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    /* ------------------------------------------------------------ *
       Clean up
     * ------------------------------------------------------------ */

    glfwDestroyWindow(window);
    glfwTerminate();

    for (uint32_t i = 0; i < swapChainFramebuffers.size(); i++)
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    for (uint32_t i = 0; i < swapChainImageViews.size(); i++)
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroyShaderModule(device, fshModule, nullptr);
    vkDestroyShaderModule(device, vshModule, nullptr);
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);

    if (enableValidationLayer)
    {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkDestroyDebugReportCallbackEXT");
        if (func)
            func(instance, callback, nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    return EXIT_SUCCESS;
}

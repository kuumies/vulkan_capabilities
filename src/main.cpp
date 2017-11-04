/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Vulkan test application.
 * ---------------------------------------------------------------- */

#include <algorithm>
#include <iostream>
#include <vector>
#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>

int main()
{
    /* ------------------------------------------------------------ *
       Vulkan GLFW window.
     * ------------------------------------------------------------ */

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(
        800, 600, "Vulkan test", nullptr, nullptr);

    /* ------------------------------------------------------------ *
       Vulkan extensions.
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
                      << ": Vulkan implementation does not support extension"
                      << glfwExtension
                      << std::endl;

            return EXIT_FAILURE;
        }
    }

    /* ------------------------------------------------------------ *
       Vulkan instance.
     * ------------------------------------------------------------ */

    // Let the implementation know that this application does not
    // use any famous 3rd party engine.
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "Vulkan test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "kuuEngine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    // Create the instance with extensions that GLFW requires.
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount       = 0;

    VkInstance instance;
    VkResult result = vkCreateInstance(&createInfo,
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
    std::vector<VkPhysicalDevice> devices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance,
                               &physicalDeviceCount,
                               devices.data());

    // Print the found device names.
    for (const VkPhysicalDevice& device : devices)
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        std::cout << __FUNCTION__
                  << ": "
                  << deviceProperties.deviceName
                  << std::endl;
    }

    // Auto-select the first device.
    VkPhysicalDevice physicalDevice = devices.front();

    /* ------------------------------------------------------------ *
       Vulkan graphics and present queue families
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
    int graphicsQueueFamilyIndex = -1;
    int presentQueueFamilyIndex  = -1;
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
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(2);

    queueCreateInfos[0].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].queueFamilyIndex = graphicsQueueFamilyIndex;
    queueCreateInfos[0].queueCount       = 1;
    queueCreateInfos[0].pQueuePriorities = &queuePriority;

    queueCreateInfos[1].sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[1].queueFamilyIndex = presentQueueFamilyIndex;
    queueCreateInfos[1].queueCount       = 1;
    queueCreateInfos[1].pQueuePriorities = &queuePriority;

    // Logical device with graphics and present queues.
    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo logicalDeviceCreateInfo = {};
    logicalDeviceCreateInfo.sType                 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.pQueueCreateInfos     = queueCreateInfos.data();
    logicalDeviceCreateInfo.queueCreateInfoCount  = 2;
    logicalDeviceCreateInfo.pEnabledFeatures      = &deviceFeatures;
    logicalDeviceCreateInfo.enabledExtensionCount = 0;
    logicalDeviceCreateInfo.enabledLayerCount     = 0;

    // Create the logical device.
    VkDevice logicalDevice;
    result = vkCreateDevice(physicalDevice,
                            &logicalDeviceCreateInfo,
                            nullptr,
                            &logicalDevice);
    if (result != VK_SUCCESS)
        std::cerr << __FUNCTION__
                  << ": failed to create vulkan logical device"
                  << std::endl;

    /* ------------------------------------------------------------ *
       Vulkan logical device queues
     * ------------------------------------------------------------ */

    VkQueue graphicsQueue;
    vkGetDeviceQueue(logicalDevice,
                     graphicsQueueFamilyIndex,
                     0,
                     &graphicsQueue);

    VkQueue presentQueue;
    vkGetDeviceQueue(logicalDevice,
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
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    vkDestroyInstance(instance, nullptr);

    return EXIT_SUCCESS;
}

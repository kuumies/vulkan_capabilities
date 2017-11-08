/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Instance class
 * ---------------------------------------------------------------- */

#include "vk_instance.h"

/* ---------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

/* ---------------------------------------------------------------- */

namespace kuu
{
namespace vk
{
namespace
{

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

} // anonymous namespace

/* ---------------------------------------------------------------- */

struct Instance::Data
{
    ~Data()
    {
        auto func = (PFN_vkDestroyDebugReportCallbackEXT)
            vkGetInstanceProcAddr(instance,
                                  "vkDestroyDebugReportCallbackEXT");
        if (func)
            func(instance, callback, nullptr);

        vkDestroyInstance(instance, nullptr);
    }

    VkInstance instance;
    VkDebugReportCallbackEXT callback;
    std::vector<PhysicalDevice> physicalDevices;
};

/* ---------------------------------------------------------------- */

Instance::Instance(const std::string& applicationName,
                   const std::string& engineName,
                   const std::vector<std::string>& extensions,
                   const std::vector<std::string>& layers)
    : d(std::make_shared<Data>())
{
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = engineName.c_str();
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;

    std::vector<const char*> extensionsStr;
    std::vector<const char*> layerStr;
    if (extensions.size())
    {
        for (auto& extension : extensions)
            extensionsStr.push_back(extension.c_str());

        instanceInfo.enabledExtensionCount   = uint32_t(extensions.size());
        instanceInfo.ppEnabledExtensionNames = extensionsStr.data();
    }
    else
        instanceInfo.enabledExtensionCount = 0;

    if (layers.size())
    {
        for (auto& layer : layers)
            layerStr.push_back(layer.c_str());

        instanceInfo.enabledLayerCount   = uint32_t(layers.size());
        instanceInfo.ppEnabledLayerNames = layerStr.data();
    }
    else
        instanceInfo.enabledLayerCount = 0;


    VkResult result = vkCreateInstance(&instanceInfo,
                                       nullptr,
                                       &d->instance);
    if (result != VK_SUCCESS)
        throw std::runtime_error(
            __FUNCTION__ +
                std::string(": failed to create Vulkan instance"));
}

/* ---------------------------------------------------------------- */

VkInstance Instance::handle() const
{ return d->instance; }

/* ---------------------------------------------------------------- */

std::vector<PhysicalDevice> Instance::physicalDevices(VkSurfaceKHR surface) const
{
    if (d->physicalDevices.size() > 0)
        return d->physicalDevices;

    // Get the device count.
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(d->instance,
                               &physicalDeviceCount,
                               nullptr);

    if (physicalDeviceCount == 0)
        return d->physicalDevices;

    // Get the devices.
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(d->instance,
                               &physicalDeviceCount,
                               physicalDevices.data());

    for (VkPhysicalDevice device : physicalDevices)
        d->physicalDevices.push_back(PhysicalDevice(device, surface));

    return d->physicalDevices;
}

/* ---------------------------------------------------------------- */

void Instance::setValidationLeyerEnabled()
{
    VkDebugReportCallbackCreateInfoEXT debugInfo = {};
    debugInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugInfo.flags        = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    debugInfo.pfnCallback = debugCallback;

    // Validation is an extension, get the function adress.
    auto func = (PFN_vkCreateDebugReportCallbackEXT)
        vkGetInstanceProcAddr(d->instance,
                              "vkCreateDebugReportCallbackEXT");

    VkResult result;
    if (func)
        result = func(d->instance, &debugInfo, nullptr, &d->callback);
    else
        result = VK_ERROR_INITIALIZATION_FAILED;
    if (result != VK_SUCCESS)
        throw std::runtime_error(
            __FUNCTION__ +
                std::string(": failed to create vulkan debug report callback"));
}

/* ---------------------------------------------------------------- */

bool Instance::isExtensionSupported(const std::string& extension)
{
    // Get the extensions count.
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           nullptr);
    if (extensionCount == 0)
        return false;

    // Get the extensions.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           extensions.data());

    const auto it = std::find_if(
        extensions.begin(),
        extensions.end(),
        [extension](const VkExtensionProperties& ex)
    {
        return std::string(ex.extensionName) ==
               std::string(extension);
    });

    return it != extensions.end();
}

/* ---------------------------------------------------------------- */

bool Instance::isLayerSupported(const std::string& layer)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (layerCount == 0)
        return false;

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount,
                                       layers.data());

    const auto it = std::find_if(
        layers.begin(),
        layers.end(),
        [layer](const VkLayerProperties& l)
    {
        return std::string(l.layerName) ==
               std::string(layer);
    });

    return it != layers.end();
}

} // namespace vk
} // namespace kuu

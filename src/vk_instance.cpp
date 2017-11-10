/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Instance class
 * ---------------------------------------------------------------- */
 
#include "vk_instance.h"

/* ---------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

namespace kuu
{
namespace vk
{
namespace
{

/* ---------------------------------------------------------------- *
   Implementation of vkCreateDebugReportCallback. This is an
   extension function whose adress needs to be retrieved.
 * ---------------------------------------------------------------- */
VkResult vkCreateDebugReportCallback(
    VkInstance instance,
    const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugReportCallbackEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)
        vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugReportCallbackEXT");

    VkResult result = VK_ERROR_INITIALIZATION_FAILED;
    if (func)
        result = func(instance, pCreateInfo, pAllocator, pCallback);
    return result;
}

/* ---------------------------------------------------------------- *
   Implementation of vkDestroyDebugReportCallback. This is an
   extension function whose adress needs to be retrieved.
 * ---------------------------------------------------------------- */
void vkDestroyDebugReportCallback(
    VkInstance instance,
    VkDebugReportCallbackEXT callback,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)
        vkGetInstanceProcAddr(instance,
                              "vkDestroyDebugReportCallbackEXT");
    if (func)
        func(instance, callback, pAllocator);
}

/* ---------------------------------------------------------------- *
   Returns true if the extension list contains the extension.
 * ---------------------------------------------------------------- */
bool containsExtension(
    const std::string& extension,
    const std::vector<std::string>& extensions)
{
    for (const std::string& ex : extensions)
        if (ex == extension)
            return true;
    return false;
}

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

/* ---------------------------------------------------------------- */
    
std::vector<const char*> toConstCharVector(
    const std::vector<std::string>& strings)
{
    std::vector<const char*> out;
    for (const std::string& string : strings)
        out.push_back(string.c_str());
    return out;
}
    
} // anonymous namespace

/* ---------------------------------------------------------------- */

struct Instance::Data
{
    Data(const Parameters& params)
    {
        //////////////////////////////////////////////////////////////
        // 1) Collect extensions and layers that user wants to use.

        std::vector<std::string> layers;
        std::vector<std::string> extensions;

        if (params.createSurfaceExtensesions)
        {
            if (!Surface::areExtensionsSupported())
            {
                std::cerr << __FUNCTION__
                          << ": surface extensions are not supported"
                          << std::endl;
                return;
            }

            extensions = Surface::extensions();
        }

        if (params.createValidationLayer)
        {
            const bool validationLayerSupported =
                Instance::isLayerSupported(
                    "VK_LAYER_LUNARG_standard_validation");
            if (validationLayerSupported)
            {
                extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
                layers.push_back("VK_LAYER_LUNARG_standard_validation");
            }
            else
            {
                std::cerr << __FUNCTION__
                          << ": validation layer is not supported"
                          << std::endl;
                return;
            }
        }

        //////////////////////////////////////////////////////////////////
        // 2) create the instance

        layers.insert(layers.end(), params.layers.begin(), params.layers.end());
        extensions.insert(extensions.end(), params.extensions.begin(), params.extensions.end());

        std::vector<const char*> extensionsAsConstChar = toConstCharVector(extensions);
        std::vector<const char*> layersAsConstChar     = toConstCharVector(layers);

        VkApplicationInfo appInfo = {};
        appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName   = params.applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName        = params.engineName.c_str();
        appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion         = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceInfo = {};
        instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceInfo.pApplicationInfo        = &appInfo;
        instanceInfo.enabledLayerCount       = uint32_t(layers.size());
        instanceInfo.enabledExtensionCount   = uint32_t(extensions.size());

        if (extensions.size())
            instanceInfo.ppEnabledExtensionNames = extensionsAsConstChar.data();
        if (layers.size())
            instanceInfo.ppEnabledLayerNames = layersAsConstChar.data();

        const VkResult result = vkCreateInstance(
            &instanceInfo,
            nullptr,
            &instance);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create Vulkan instance"
                      << std::endl;
            return;
        }

        //////////////////////////////////////////////////////////////////
        // 3) create the physical devices

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(
            instance,
            &physicalDeviceCount,
            nullptr);

        if (physicalDeviceCount > 0)
        {
            std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
            vkEnumeratePhysicalDevices(
                instance,
                &physicalDeviceCount,
                physicalDevices.data());

            for (VkPhysicalDevice device : physicalDevices)
                this->physicalDevices.push_back(PhysicalDevice(device));
        }
        else
        {
            std::cerr << __FUNCTION__
                      << ": no physical devices with Vulkan support"
                      << std::endl;
            return;
        }

        //////////////////////////////////////////////////////////////////
        // 4) create the debug callback if user has set the debug report
        //    extensions and it is supported by system.

        if (params.createValidationLayer)
        {
            VkDebugReportCallbackCreateInfoEXT debugInfo = {};
            debugInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debugInfo.flags        = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugInfo.pfnCallback  = debugCallback;

            vkCreateDebugReportCallback(
                instance,
                &debugInfo,
                nullptr,
                &callback);
        }

        valid = true;
    }

    ~Data()
    {
        if (!valid)
            return;

        if (callback)
            vkDestroyDebugReportCallback(
                instance,
                callback,
                nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    VkInstance instance;
    bool valid;
    
    std::vector<PhysicalDevice> physicalDevices;

    VkDebugReportCallbackEXT callback;
};

/* ---------------------------------------------------------------- */

Instance::Instance(const Parameters& params)
    : d(std::make_shared<Data>(params))
{}

/* ---------------------------------------------------------------- */

bool Instance::isValid() const
{ return d->valid; }

/* ---------------------------------------------------------------- */

VkInstance Instance::handle() const
{ return d->instance; }

/* ---------------------------------------------------------------- */

Surface Instance::createSurface(GLFWwindow* window) const
{ return Surface(*this, window); }

/* ---------------------------------------------------------------- */

std::vector<PhysicalDevice> Instance::physicalDevices() const
{ return d->physicalDevices; }

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

/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Instance class
 * -------------------------------------------------------------------------- */

#include "vk_instance.h"

/* -------------------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

/* -------------------------------------------------------------------------- */

#include "vk_stringify.h"

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

/* -------------------------------------------------------------------------- */

std::vector<VkExtensionProperties> getExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(
        NULL,            // [in]  NULL -> Vulkan implementation extensions
        &extensionCount, // [out] Extensions count
        NULL);           // [out] NULL -> Get only count

    std::vector<VkExtensionProperties> extensions;
    if (extensionCount)
    {
        extensions.resize(extensionCount);
        vkEnumerateInstanceExtensionProperties(
            nullptr,            // [in]  NULL -> Vulkan implementation extensions
            &extensionCount,    // [in, out] Extensions count
            extensions.data()); // [out] Extensions
    }

    return extensions;
}

/* -------------------------------------------------------------------------- */

std::vector<VkLayerProperties> getLayers()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(
        &layerCount,
        nullptr);

    std::vector<VkLayerProperties> layers;
    if (layerCount > 0)
    {
        layers.resize(layerCount);
        vkEnumerateInstanceLayerProperties(
            &layerCount,
            layers.data());
    }

    return layers;
}

/* -------------------------------------------------------------------------- */

std::vector<PhysicalDevice> getPhysicalDevices(const Instance& instance)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(
        instance.instance,    // [in]  Instance handle
        &physicalDeviceCount, // [out] Physical device count
        NULL);                // [in]  Pointer to vector of physical devices, NULL
                              // so the physical device count is returned.

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VkResult result = vkEnumeratePhysicalDevices(
        instance.instance,       // [in]      Instance handle
        &physicalDeviceCount,    // [in, out] Physical device count
        physicalDevices.data()); // [out]     Pointer to vector of physical devices

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": physical device enumeration failed as "
                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return std::vector<PhysicalDevice>();
    }

    std::vector<PhysicalDevice> out;
    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
        out.push_back(PhysicalDevice(physicalDevice, instance));
    return out;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

Instance::Instance(
    const std::vector<std::string>& extensions,
    bool validate)
{
    std::vector<const char*> extensionNames;
    for (const std::string& extension : extensions)
        extensionNames.push_back(extension.c_str());
    
    std::vector<const char*> layerNames;
    
    if (validate)
    {
        const bool validationLayerSupported =
            isLayerSupported(
                "VK_LAYER_LUNARG_standard_validation");
        if (validationLayerSupported)
        {
            extensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            layerNames.push_back("VK_LAYER_LUNARG_standard_validation");
        }
        else
        {
            std::cerr << __FUNCTION__
                      << ": validation layer is not supported"
                      << std::endl;
            return;
        }
    }
    
    const uint32_t extensionCount =
        static_cast<uint32_t>(extensionNames.size());

    VkStructureType appInfoType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    VkApplicationInfo appInfo;
    appInfo.sType              = appInfoType;              // Must be this definition
    appInfo.pNext              = NULL;                     // No extension specific structures
    appInfo.pApplicationName   = "Vulkan Capabilities";    // Name of the application
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version of the application
    appInfo.pEngineName        = "CapabilitiesEngine";     // Name of the "engine"
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0); // Version of the "engine"
    appInfo.apiVersion         = VK_API_VERSION_1_0;       // Version of the Vulkan API

    VkStructureType infoType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    VkInstanceCreateInfo info;
    info.sType                   = infoType;              // Must be this definition
    info.pNext                   = NULL;                  // No extension specific structures
    info.flags                   = 0;                     // Must be 0, reserved for future use
    info.pApplicationInfo        = &appInfo;              // Application information
    info.enabledLayerCount       = layerNames.size();     // Count of requested layers
    info.ppEnabledLayerNames     = layerNames.data();     // Requested layer names
    info.enabledExtensionCount   = extensionCount;        // Count of requested extensions
    info.ppEnabledExtensionNames = extensionNames.data(); // Requested extension names

    VkResult result = vkCreateInstance(
        &info,      // [in]  Instance create info
        NULL,       // [in]  No allocation callback
        &instance); // [out] Instance handle

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": instance creation failed as "
                  << vk::stringify::result(result)
                << std::endl;
        return;
    }
    
    if (validate)
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

    this->extensionNames      = extensions;
    this->availableExtensions = getExtensions();
    this->availableLayers     = getLayers();
    this->physicalDevices     = getPhysicalDevices(*this);
}

/* -------------------------------------------------------------------------- */

Instance::~Instance()
{
    if (callback)
        vkDestroyDebugReportCallback(
            instance,
            callback,
            nullptr);
    vkDestroyInstance(
        instance, // [in] Handle to instance, can be a VK_NULL_HANDLE
                NULL);    // [in] No allocation callback
}

/* -------------------------------------------------------------------------- */

bool Instance::isExtensionSupported(const std::string& extension)
{
    std::vector<VkExtensionProperties> extensions = getExtensions();
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
    const std::vector<VkLayerProperties> layers = getLayers();

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

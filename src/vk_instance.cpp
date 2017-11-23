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
                  << vk::stringify::toDescription(result)
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

Instance::Instance(const std::vector<std::string>& extensions)
{
    std::vector<const char*> extensionNames;
    for (const std::string& extension : extensions)
        extensionNames.push_back(extension.c_str());
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
    info.enabledLayerCount       = 0;                     // Count of requested layers
    info.ppEnabledLayerNames     = NULL;                  // Requested layer names
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
                  << vk::stringify::toString(result)
                << std::endl;
        return;
    }

    this->extensionNames      = extensions;
    this->availableExtensions = getExtensions();
    this->availableLayers              = getLayers();
    this->physicalDevices     = getPhysicalDevices(*this);
}

/* -------------------------------------------------------------------------- */

Instance::~Instance()
{
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

} // namespace vk
} // namespace kuu

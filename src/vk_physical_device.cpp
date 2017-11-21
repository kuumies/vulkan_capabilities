/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::PhysicalDevice class
 * -------------------------------------------------------------------------- */

#include "vk_physical_device.h"

/* -------------------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

/* -------------------------------------------------------------------------- */

#include "vk_instance.h"
#include "vk_stringify.h"

#ifdef _WIN32
    #include "vk_windows.h"
#endif

namespace kuu
{
namespace vk
{
namespace
{

/* -------------------------------------------------------------------------- */

VkPhysicalDeviceProperties getProperties(const VkPhysicalDevice& physicalDevice)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(
        physicalDevice, // [in]  physical device handle
        &properties);   // [out] physical device properties
    return properties;
}

/* -------------------------------------------------------------------------- */

VkPhysicalDeviceFeatures getFeatures(const VkPhysicalDevice& physicalDevice)
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(
        physicalDevice, // [in]  physical device handle
        &features);     // [out] physical device features
    return features;
}

/* -------------------------------------------------------------------------- */

VkPhysicalDeviceMemoryProperties getMemoryProperties(
    const VkPhysicalDevice& physicalDevice)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(
        physicalDevice,     // [in]  physical device handle
        &memoryProperties); // [out] memory properties
    return memoryProperties;
}

/* -------------------------------------------------------------------------- */

std::vector<std::pair<VkFormat, VkFormatProperties>> getFormats(
    const VkPhysicalDevice& physicalDevice)
{
    std::vector<std::pair<VkFormat, VkFormatProperties>> formats;
    for (int f = VK_FORMAT_R4G4_UNORM_PACK8;
         f <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
         ++f)
    {
        VkFormat fmt = VkFormat(f);
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(
            physicalDevice, // [in]  physical device handle
            fmt,            // [in]  format
            &props);        // [out] format properties
        formats.push_back( { fmt, props} );
    }
    return formats;
}

std::vector<VkQueueFamilyProperties> getQueueFamilies(
    const VkPhysicalDevice& physicalDevice)
{
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,            // [in]  physical device handle
        &queueFamilyPropertyCount, // [out] queue family property count
        NULL);                     // [in]  properties, NULL to get count

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,                // [in]  physical device handle
        &queueFamilyPropertyCount,     // [in]  queue family property count
        queueFamilyProperties.data()); // [out] queue family properties

    return queueFamilyProperties;
}

std::vector<bool> getQueuePresentationSupports(
    const VkInstance& instance,
    const VkPhysicalDevice& physicalDevice,
    const std::vector<VkQueueFamilyProperties>& queueProperties)
{
    std::vector<bool> presentationSupports;
    for (uint32_t queueFamilyIndex = 0;
         queueFamilyIndex < queueProperties.size();
         ++queueFamilyIndex)
    {
#ifdef _WIN32
        using namespace windows;
        VkBool32 result = vkGetPhysicalDeviceWin32PresentationSupportKHR(
            instance,
            physicalDevice,  // [in] physical device handle
            queueFamilyIndex);
        presentationSupports.push_back(result == VK_TRUE);
#else
        std::cerr << __FUNCTION__ << "Unsupported OS" << std::endl;
        presentationSupports.push_back(false);
#endif
    }

    return presentationSupports;
}

/* -------------------------------------------------------------------------- */

std::vector<VkExtensionProperties> getExtensions(
    const VkPhysicalDevice& physicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(
        physicalDevice,   // [in]  physical device handle
        NULL,             // [in]  NULL -> implementation extensions
        &extensionCount,  // [out] Count of physical device extensions.
        NULL);            // [in]  NULL -> get only count.

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        physicalDevice,      // [in]  physical device handle
        NULL,                // [in]  NULL -> implementation extensions
        &extensionCount,     // [out] Count of physical device extensions.
        extensions.data());  // [in]  Physical device extensions
    return extensions;
}

/* -------------------------------------------------------------------------- */

bool getFeatures2(
    const VkInstance& instance,
    const VkPhysicalDevice& physicalDevice,
    VkPhysicalDeviceVariablePointerFeaturesKHR& featuresVariablePointer,
    VkPhysicalDeviceMultiviewFeaturesKHX& multiviewFeatures,
    VkPhysicalDevice16BitStorageFeaturesKHR& features16BitStorage,
    VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR& yuvSamplerFeatures,
    VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT& blendFeatures)
{
    auto fun = (PFN_vkGetPhysicalDeviceFeatures2KHR)
        vkGetInstanceProcAddr(
            instance,
            "vkGetPhysicalDeviceFeatures2KHR");
    if (!fun)
        return false;

    // Query SPIR-V VariablePointers and VariablePointersStorageBuffer capabilities.
    featuresVariablePointer.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR;
    featuresVariablePointer.pNext = NULL; // end of the chain, must be NULL!

    // Query render pass mutltiview capablities
    multiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHX;
    multiviewFeatures.pNext = &featuresVariablePointer;

    // Query storage 16 bit capabilities
    features16BitStorage.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
    features16BitStorage.pNext = &multiviewFeatures;

    // Query samper Yuv conversion capability
    yuvSamplerFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR;
    yuvSamplerFeatures.pNext = &features16BitStorage;

    // Query advanced blending operation capability
    blendFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
    blendFeatures.pNext = &yuvSamplerFeatures;

    // Get the features of Vulkan 1.0 and beyond API
    VkPhysicalDeviceFeatures2KHR features2;
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
    features2.pNext = &blendFeatures;
    fun(physicalDevice, &features2);

    return true;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& physicalDevice,
                               const Instance& instance)
    : physicalDevice(physicalDevice)
{
    properties        = getProperties(physicalDevice);
    features          = getFeatures(physicalDevice);
    memoryProperties  = getMemoryProperties(physicalDevice);
    queueFamilies     = getQueueFamilies(physicalDevice);
    queuePresentation = getQueuePresentationSupports(instance.instance, physicalDevice, queueFamilies);
    formats           = getFormats(physicalDevice);
    extensions        = getExtensions(physicalDevice);

    const auto it = std::find_if(
        instance.extensionNames.begin(),
        instance.extensionNames.end(),
        [](const std::string& ex)
    { return ex == std::string("VK_KHR_get_physical_device_properties2"); });

    if (it != instance.extensionNames.end())
    {
        getFeatures2(instance.instance,
                     physicalDevice,
                     featuresVariablePointer,
                     multiviewFeatures,
                     features16BitStorage,
                     yuvSamplerFeatures,
                     blendFeatures);
    }
}

} // namespace vk_capabilities
} // namespace kuu

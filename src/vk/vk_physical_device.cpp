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

bool getProperties2(
    const VkInstance& instance,
    const VkPhysicalDevice& physicalDevice,
    VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT& blendProperties,
    VkPhysicalDeviceDiscardRectanglePropertiesEXT&  discardRectangleProperties,
    VkPhysicalDeviceIDPropertiesKHR&  idProperties,
    VkPhysicalDeviceMultiviewPropertiesKHX&  multiviewProperties,
    VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& multiviewPerView,
    VkPhysicalDevicePointClippingPropertiesKHR& clippingProperties,
    VkPhysicalDevicePushDescriptorPropertiesKHR&  pushDescriptorProperties,
    VkPhysicalDeviceSampleLocationsPropertiesEXT& sampleLocationsProperties,
    VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT& samplerMinMaxProperties)
{
    auto fun = (PFN_vkGetPhysicalDeviceProperties2KHR)
        vkGetInstanceProcAddr(
            instance,
            "vkGetPhysicalDeviceProperties2KHR");
    if (!fun)
        return false;
    idProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES_KHR;
    idProperties.pNext = NULL; // must be NULL

    VkPhysicalDeviceProperties2KHR properties2;
    properties2.pNext = &idProperties;
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
    fun(physicalDevice, &properties2);


    multiviewProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PROPERTIES_KHX;
    multiviewProperties.pNext = NULL; // must be NULL
    properties2 = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR, &multiviewProperties };
    //properties2.pNext = &multiviewProperties;
    fun(physicalDevice, &properties2);

    multiviewPerView.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX;
    multiviewPerView.pNext = NULL;
    properties2.pNext = &multiviewPerView;
    fun(physicalDevice, &properties2);

    clippingProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_POINT_CLIPPING_PROPERTIES_KHR;
    clippingProperties.pNext = NULL;
    properties2.pNext = &clippingProperties;
    fun(physicalDevice, &properties2);

    discardRectangleProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISCARD_RECTANGLE_PROPERTIES_EXT;
    discardRectangleProperties.pNext = NULL;
    properties2.pNext = &discardRectangleProperties;
    fun(physicalDevice, &properties2);

    sampleLocationsProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLE_LOCATIONS_PROPERTIES_EXT;
    sampleLocationsProperties.pNext = NULL;
    properties2.pNext = &sampleLocationsProperties;
    fun(physicalDevice, &properties2);

    blendProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_PROPERTIES_EXT;
    blendProperties.pNext = NULL;
    properties2.pNext = &blendProperties;
    fun(physicalDevice, &properties2);

    samplerMinMaxProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT;
    samplerMinMaxProperties.pNext = NULL;
    properties2.pNext = &samplerMinMaxProperties;
    fun(physicalDevice, &properties2);

    pushDescriptorProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR;
    pushDescriptorProperties.pNext = NULL;
    properties2.pNext = &pushDescriptorProperties;
    fun(physicalDevice, &properties2);

    return true;
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

PhysicalDeviceInfo::PhysicalDeviceInfo(
    const VkPhysicalDevice& physicalDevice,
    const VkInstance& instance)
{
    properties        = getProperties(physicalDevice);
    features          = getFeatures(physicalDevice);
    memoryProperties  = getMemoryProperties(physicalDevice);
    queueFamilies     = getQueueFamilies(physicalDevice);
    queuePresentation = getQueuePresentationSupports(instance, physicalDevice, queueFamilies);
    formats           = getFormats(physicalDevice);
    extensions        = getExtensions(physicalDevice);

    hasExtensionsFeatures = getFeatures2(
        instance,
        physicalDevice,
        featuresVariablePointer,
        multiviewFeatures,
        features16BitStorage,
        yuvSamplerFeatures,
        blendFeatures);

    hasExtensionsProperties = getProperties2(
        instance,
        physicalDevice,
        blendProperties,
        discardRectangleProperties,
        idProperties,
        multiviewProperties,
        multiviewPerView,
        clippingProperties,
        pushDescriptorProperties,
        sampleLocationsProperties,
        samplerMinMaxProperties);
}

/* -------------------------------------------------------------------------- *
   Implementation of the physical device.
 * -------------------------------------------------------------------------- */
struct PhysicalDevice::Impl
{
    /* ---------------------------------------------------------------------- *
       Constructs the implementation.
    * ----------------------------------------------------------------------- */
    Impl(const VkPhysicalDevice& physicalDevice,
         const VkInstance& instance)
        : physicalDevice(physicalDevice)
        , instance(instance)
        , info(physicalDevice, instance)
        , features(info.features)
    {}

    /* ---------------------------------------------------------------------- *
       Destroys the logical device if it is created and has not been 
       destroyed.
    * ----------------------------------------------------------------------- */
    ~Impl()
    {
        destroy();
    }

    /* ---------------------------------------------------------------------- *
       Creates the logical device.
    * ----------------------------------------------------------------------- */
    void create()
    {
        // Fill the queue create infos.
        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        for (size_t i = 0; i < queueFamilyParams.size(); ++i)
        {
            const QueueFamilyParams& params = queueFamilyParams[i];

            VkStructureType type = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            VkDeviceQueueCreateInfo queueInfo;
            queueInfo.sType            = type;                    // Type of the struct.
            queueInfo.queueFamilyIndex = params.queueFamilyIndex; // Queue family index.
            queueInfo.queueCount       = params.queueCount;       // Count of queues.
            queueInfo.pQueuePriorities = &params.priority;        // Priority of the queue.
            queueInfo.pNext            = NULL;                    // No extension usage
            queueInfo.flags            = 0;                       // Must be 0.

            queueInfos.push_back(queueInfo);
        }

        // Extensions
        std::vector<const char*> extensionNamesStr;
        for (auto& extension : extensions)
            extensionNamesStr.push_back(extension.c_str());

        // Layers
        std::vector<const char*> layerNamesStr;
        for (auto& layer : layers)
            layerNamesStr.push_back(layer.c_str());

        // Fill create info
        VkDeviceCreateInfo deviceInfo = {};
        deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; // Type of the struct.
        deviceInfo.pNext                   = NULL;                                 // Extension chain.
        deviceInfo.flags                   = 0;                                    // Must be 0.
        deviceInfo.queueCreateInfoCount    = uint32_t(queueInfos.size());          // Queue info count.
        deviceInfo.pQueueCreateInfos       = queueInfos.data();                    // Queue infos.
        deviceInfo.enabledLayerCount       = uint32_t(layerNamesStr.size());       // Layer count.
        deviceInfo.ppEnabledLayerNames     = layerNamesStr.data();                 // Layer names.
        deviceInfo.enabledExtensionCount   = uint32_t(extensionNamesStr.size());   // Extension count.
        deviceInfo.ppEnabledExtensionNames = extensionNamesStr.data();             // Extension names.
        deviceInfo.pEnabledFeatures        = &features;                            // Features to enable.
        
        // Create the logical device.
        const VkResult result = vkCreateDevice(
            physicalDevice,     // [in]  Physical device handle
            &deviceInfo,        // [in]  Device info
            NULL,               // [in]  Allocator
            &logicalDevice);    // [out] Handle to logical device opaque object

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create logical device"
                      << std::endl;
        }
    }

   /* ----------------------------------------------------------------------- *
       Destroys the logical device.
    * ----------------------------------------------------------------------- */
    void destroy()
    {
        vkDestroyDevice(
            logicalDevice, // [in] Logical device
            NULL);         // [in] Allocator

        logicalDevice = VK_NULL_HANDLE;
    }

    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice = VK_NULL_HANDLE;
    std::vector<QueueFamilyParams> queueFamilyParams;
    std::vector<std::string> extensions;
    std::vector<std::string> layers;
    PhysicalDeviceInfo info;
    VkPhysicalDeviceFeatures features;
};

/* -------------------------------------------------------------------------- *
   Constructs the physical device instance.
 * -------------------------------------------------------------------------- */
PhysicalDevice::PhysicalDevice(const VkPhysicalDevice& physicalDevice,
                               const VkInstance& instance)
    : impl(std::make_shared<Impl>(physicalDevice, instance))
{}

/* -------------------------------------------------------------------------- *
   Sets the logical device extensions.
 * -------------------------------------------------------------------------- */
PhysicalDevice& PhysicalDevice::setExtensions(
    const std::vector<std::string>& extensions)
{
    impl->extensions = extensions;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the logical device extensions.
 * -------------------------------------------------------------------------- */
std::vector<std::string> PhysicalDevice::extensions() const
{ return impl->extensions; }

/* -------------------------------------------------------------------------- *
   Sets the logical device layers.
 * -------------------------------------------------------------------------- */
PhysicalDevice& PhysicalDevice::setLayers(
    const std::vector<std::string>& layers)
{
    impl->layers = layers;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the logical device layers.
 * -------------------------------------------------------------------------- */
std::vector<std::string> PhysicalDevice::layers() const
{ return impl->layers; }

/* -------------------------------------------------------------------------- *
   Sets the device features.
 * -------------------------------------------------------------------------- */
PhysicalDevice& PhysicalDevice::setFeatures(
    const VkPhysicalDeviceFeatures& features)
{ 
    impl->features = features; 
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the device features.
 * -------------------------------------------------------------------------- */
VkPhysicalDeviceFeatures PhysicalDevice::features() const
{ return impl->features; }

/* -------------------------------------------------------------------------- *
   Adds a queue family to be created.
 * -------------------------------------------------------------------------- */
PhysicalDevice& PhysicalDevice::addQueueFamily(
        const QueueFamilyParams& queue)
{
    impl->queueFamilyParams.push_back(queue);
    return *this;
}

/* -------------------------------------------------------------------------- *
   Adds a queue family to be created.
 * -------------------------------------------------------------------------- */
PhysicalDevice& PhysicalDevice::addQueueFamily(
    uint32_t queueFamilyIndex,
    uint32_t queueCount,
    float priority)
{
    return addQueueFamily(
        QueueFamilyParams({
            queueFamilyIndex, 
            queueCount, 
            priority}));
}

/* -------------------------------------------------------------------------- *
   Returns the queue family parameters.
 * -------------------------------------------------------------------------- */
std::vector<PhysicalDevice::QueueFamilyParams>
    PhysicalDevice::queueFamilyParams() const
{ return impl->queueFamilyParams; }

/* -------------------------------------------------------------------------- *
   Creates the logical device.
 * -------------------------------------------------------------------------- */
void PhysicalDevice::create()
{
    if (!isValid()) 
        impl->create();
}

/* -------------------------------------------------------------------------- *
   Destroys the logical device.
 * -------------------------------------------------------------------------- */
void PhysicalDevice::destroy()
{
    if (isValid()) 
        impl->destroy(); 
}

/* -------------------------------------------------------------------------- *
   Returns the physical device handle.
 * -------------------------------------------------------------------------- */
bool PhysicalDevice::isValid() const
{ return impl->logicalDevice != VK_NULL_HANDLE; }

/* -------------------------------------------------------------------------- *
   Returns the info of physical device.
 * -------------------------------------------------------------------------- */
PhysicalDeviceInfo PhysicalDevice::info() const
{ return impl->info; }

/* -------------------------------------------------------------------------- *
   Returns the physical device handle.
 * -------------------------------------------------------------------------- */
VkPhysicalDevice PhysicalDevice::physicalDeviceHandle() const
{ return impl->physicalDevice; }

/* -------------------------------------------------------------------------- *
   Returns the logical device handle.
 * -------------------------------------------------------------------------- */
VkDevice PhysicalDevice::logicalDeviceHandle() const
{ return impl->logicalDevice; }

} // namespace vk
} // namespace kuu

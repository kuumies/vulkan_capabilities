/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_capabilities_controller.h"

/* -------------------------------------------------------------------------- */

#include <iostream>
#include <sstream>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_data.h"
#include "vk_capabilities_main_window.h"
#include "vk_helper.h"
#include "vk_stringify.h"

namespace kuu
{
namespace vk_capabilities
{
namespace
{

/* ---------------------------------------------------------------- */

QStringList readDescription(const QString& filepath)
{
    QFile qssFile(filepath);
    if (!qssFile.open(QIODevice::ReadOnly))
    {
        std::cerr << __FUNCTION__
                  << "Failed to read desciptions from "
                  << filepath.toStdString()
                  << std::endl;
        return QStringList();
    }

    QStringList out;
    QTextStream ts(&qssFile);
    while(!ts.atEnd())
    {
        QString line = ts.readLine().trimmed();
        if (line.startsWith(";"))
            continue;
        if (line.isEmpty())
            continue;

        out.push_back(line);
    }

    return out;
}


/* -------------------------------------------------------------------------- *
   Maintains (creates, destroys) the Vulkan objects.
 * -------------------------------------------------------------------------- */
struct VulkanObjects
{
    /* ---------------------------------------------------------------------- *
       Defines a physical device data struct.
     * ---------------------------------------------------------------------- */
    struct PhysicalDevice
    {
        const VkPhysicalDevice physicalDevice;                                      // Handle
        const VkPhysicalDeviceProperties properties;                                // Properties
        const VkPhysicalDeviceFeatures features;                                    // Features
        const VkPhysicalDeviceFeatures2KHR features2;                               // Features2
        const VkPhysicalDeviceVariablePointerFeaturesKHR featuresVariablePointer;   // Variable pointer
        const VkPhysicalDeviceMultiviewFeaturesKHX multiviewFeatures;               // Multiview
        const VkPhysicalDevice16BitStorageFeaturesKHR features16ButStorage;         // 16 bit storage
        const VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR yuvSamplerFeatures; // YUV sampler
        const VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blendFeatures;      // Blend features
        const std::vector<VkExtensionProperties> extensions;                        // Extensions
    };

    VulkanObjects()
    {
        /* ------------------------------------------------------------------ *
           Create instance
         * ------------------------------------------------------------------ */

        physicalDeviceProperties2 =
            vk::helper::isInstanceExtensionSupported(
                "VK_KHR_get_physical_device_properties2");

        std::vector<const char*> extensionNames;
        if (physicalDeviceProperties2)
            extensionNames.push_back("VK_KHR_get_physical_device_properties2");
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

        /* ------------------------------------------------------------------ *
           Get the instance extensions.
         * ------------------------------------------------------------------ */

        uint32_t instanceExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(
            NULL,                    // [in]  NULL -> Vulkan implementation extensions
            &instanceExtensionCount, // [out] Extensions count
            NULL);                   // [out] NULL -> Get only count
        if (instanceExtensionCount)
        {
            instanceExtensions.resize(instanceExtensionCount);
            vkEnumerateInstanceExtensionProperties(
                nullptr,                    // [in]  NULL -> Vulkan implementation extensions
                &instanceExtensionCount,    // [in, out] Extensions count
                instanceExtensions.data()); // [out] Extensions
        }

        /* ------------------------------------------------------------------ *
           Get the instance layers.
         * ------------------------------------------------------------------ */

        uint32_t instanceLayerCount;
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        if (instanceLayerCount > 0)
        {
            instanceLayers.resize(instanceLayerCount);
            vkEnumerateInstanceLayerProperties(&instanceLayerCount,
                                               instanceLayers.data());
        }

        /* ------------------------------------------------------------------ *
           Enumerate physical devices
         * ------------------------------------------------------------------ */

        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(
            instance,             // [in]  Instance handle
            &physicalDeviceCount, // [out] Physical device count
            NULL);                // [in]  Pointer to vector of physical devices, NULL
                                  // so the physical device count is returned.

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        result = vkEnumeratePhysicalDevices(
            instance,                // [in]      Instance handle
            &physicalDeviceCount,    // [in, out] Physical device count
            physicalDevices.data()); // [out]     Pointer to vector of physical devices

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": physical device enumeration failed as "
                      << vk::stringify::toDescription(result)
                      << std::endl;
            return;
        }

        /* ------------------------------------------------------------------ *
           Retrieve per-physical device data.
         * ------------------------------------------------------------------ */

        for (const VkPhysicalDevice& physicalDevice : physicalDevices)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(
                physicalDevice, // [in]  physical device handle
                &properties);   // [out] physical device properties

            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(
                physicalDevice,
                &features);

            VkPhysicalDeviceVariablePointerFeaturesKHR featuresVariablePointer;
            VkPhysicalDeviceMultiviewFeaturesKHX multiviewFeatures;
            VkPhysicalDevice16BitStorageFeaturesKHR features16ButStorage;
            VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR yuvSamplerFeatures;
            VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blendFeatures;
            VkPhysicalDeviceFeatures2KHR features2;

            if (physicalDeviceProperties2)
            {
                auto fun = (PFN_vkGetPhysicalDeviceFeatures2KHR)
                    vkGetInstanceProcAddr(
                        instance,
                        "vkGetPhysicalDeviceFeatures2KHR");

                if (fun)
                {
                    // Query SPIR-V VariablePointers and VariablePointersStorageBuffer capabilities.
                    featuresVariablePointer.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTER_FEATURES_KHR;
                    featuresVariablePointer.pNext = NULL; // end of the chain, must be NULL!

                    // Query render pass mutltiview capablities
                    multiviewFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES_KHX;
                    multiviewFeatures.pNext = &featuresVariablePointer;

                    // Query storage 16 bit capabilities
                    features16ButStorage.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES_KHR;
                    features16ButStorage.pNext = &multiviewFeatures;

                    // Query samper Yuv conversion capability
                    yuvSamplerFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES_KHR;
                    yuvSamplerFeatures.pNext = &features16ButStorage;

                    // Query advanced blending operation capability
                    blendFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;
                    blendFeatures.pNext = &yuvSamplerFeatures;

                    // Get the features of Vulkan 1.0 and beyond API
                    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
                    features2.pNext = &blendFeatures;
                    fun(physicalDevice, &features2);
                }
            }

            uint32_t devInstanceCount = 0;
            vkEnumerateDeviceExtensionProperties(
                physicalDevice,     // [in]  physical device handle
                NULL,               // [in]  NULL -> implementation extensions
                &devInstanceCount,  // [out] Count of physical device extensions.
                NULL);              // [in]  NULL -> get only count.

            std::vector<VkExtensionProperties> devExtensions;
            devExtensions.resize(devInstanceCount);
            vkEnumerateDeviceExtensionProperties(
                physicalDevice,        // [in]  physical device handle
                NULL,                  // [in]  NULL -> implementation extensions
                &devInstanceCount,     // [out] Count of physical device extensions.
                devExtensions.data()); // [in]  Physical device extensions

            this->physicalDevices.push_back( 
                {  
                    physicalDevice, 
                    properties,
                    features,
                    features2,
                    featuresVariablePointer,
                    multiviewFeatures,
                    features16ButStorage,
                    yuvSamplerFeatures,
                    blendFeatures,
                    devExtensions
                });
        }
    }

    ~VulkanObjects()
    {
        vkDestroyInstance(
            instance, // [in] Handle to instance, can be a VK_NULL_HANDLE
            NULL);    // [in] No allocation callback
    }

    // Vulkan instance handle. This a null handle if the system does not
    // contain a Vulkan implementation.
    VkInstance instance = VK_NULL_HANDLE;
    // Instance extensions
    std::vector<VkExtensionProperties> instanceExtensions;
    // Instance layers
    std::vector<VkLayerProperties> instanceLayers;
    // Vulkan physical device handles
    std::vector<PhysicalDevice> physicalDevices;
    // True if the VK_KHR_get_physical_device_properties2 extension is supported.
    bool physicalDeviceProperties2 = false;
};

/* -------------------------------------------------------------------------- *
   Creates the capabilities data from the Vulkan objects.
 * -------------------------------------------------------------------------- */
std::shared_ptr<Data> createCapabilitiesData(
    std::shared_ptr<VulkanObjects> vulkanObjects)
{
    using namespace vk::stringify;

    std::shared_ptr<Data> out = std::make_shared<Data>();
    if (vulkanObjects->instance == VK_NULL_HANDLE)
        return out; // No Vulkan implementation
    out->hasVulkan = true;

    for (const VkExtensionProperties& ex : vulkanObjects->instanceExtensions)
    {
        out->instanceExtensions.push_back(
            std::make_pair(ex.extensionName,
                           std::to_string(ex.specVersion)));
    }

    for (const VkLayerProperties& l : vulkanObjects->instanceLayers)
    {
        out->instanceLayers.push_back(
            { l.layerName,
              l.description,
              std::to_string(l.specVersion),
              std::to_string(l.implementationVersion) });
    }

    for (const VulkanObjects::PhysicalDevice& device : vulkanObjects->physicalDevices)
    {
        Data::PhysicalDeviceData d;
        d.mainProperties.push_back( { "Name",                device.properties.deviceName                   });
        d.mainProperties.push_back( { "Type",                toString(device.properties.deviceType)         });
        d.mainProperties.push_back( { "API Version",         versionNumber(device.properties.apiVersion)    });
        d.mainProperties.push_back( { "Driver Version",      versionNumber(device.properties.driverVersion) });
        d.mainProperties.push_back( { "Vendor ID",           hexValueToString(device.properties.vendorID)   });
        d.mainProperties.push_back( { "Device ID",           hexValueToString(device.properties.deviceID)   });
        d.mainProperties.push_back( { "Pipeline Cache UUID", toString(device.properties.pipelineCacheUUID)  });

        d.mainFeatures.push_back( { "Robust Buffer Access",   bool(device.features.robustBufferAccess) } );
        d.mainFeatures.push_back( { "Full Draw Index Uint32", bool(device.features.fullDrawIndexUint32) } );
        d.mainFeatures.push_back( { "Image Cube Array",       bool(device.features.imageCubeArray) } );
        d.mainFeatures.push_back( { "Independent Blend",      bool(device.features.independentBlend) } );
        d.mainFeatures.push_back( { "Geometry Shader",        bool(device.features.geometryShader) } );
        d.mainFeatures.push_back( { "Tesselation Shader",     bool(device.features.tessellationShader) } );
        d.mainFeatures.push_back( { "Sample Rate Shading", bool(device.features.sampleRateShading) } );
        d.mainFeatures.push_back( { "Dual SRC Blend", bool(device.features.dualSrcBlend) } );
        d.mainFeatures.push_back( { "Logic OP", bool(device.features.logicOp) } );
        d.mainFeatures.push_back( { "Multi Draw Indirect", bool(device.features.multiDrawIndirect) } );
        d.mainFeatures.push_back( { "Draw Inderect First Instance", bool(device.features.drawIndirectFirstInstance) } );
        d.mainFeatures.push_back( { "Depth CLamp", bool(device.features.depthClamp) } );
        d.mainFeatures.push_back( { "Depth Bias Clamp", bool(device.features.depthBiasClamp) } );
        d.mainFeatures.push_back( { "Fill Mode Non Solid", bool(device.features.fillModeNonSolid) } );
        d.mainFeatures.push_back( { "Depth Bounds", bool(device.features.depthBounds) } );
        d.mainFeatures.push_back( { "Wide Lines", bool(device.features.wideLines) } );
        d.mainFeatures.push_back( { "Large Points", bool(device.features.largePoints) } );
        d.mainFeatures.push_back( { "Alpha To One", bool(device.features.alphaToOne) } );
        d.mainFeatures.push_back( { "Multi Viewport", bool(device.features.multiViewport) } );
        d.mainFeatures.push_back( { "Sampler Anisotropy", bool(device.features.samplerAnisotropy) } );
        d.mainFeatures.push_back( { "Texture Compression ETC2", bool(device.features.textureCompressionETC2) } );
        d.mainFeatures.push_back( { "Texture Compression ASTC_LDR", bool(device.features.textureCompressionASTC_LDR) } );
        d.mainFeatures.push_back( { "Texture Compression BC", bool(device.features.textureCompressionBC) } );
        d.mainFeatures.push_back( { "Occlusion Query Precise", bool(device.features.occlusionQueryPrecise) } );
        d.mainFeatures.push_back( { "Pipeline Staticstics Query", bool(device.features.pipelineStatisticsQuery) } );
        d.mainFeatures.push_back( { "Vertex Pipeline Stores and Atomics", bool(device.features.vertexPipelineStoresAndAtomics) } );
        d.mainFeatures.push_back( { "Fragment Stores and Atomics", bool(device.features.fragmentStoresAndAtomics) } );
        d.mainFeatures.push_back( { "Shader Tesselation And Geometry Point Size", bool(device.features.shaderTessellationAndGeometryPointSize) } );
        d.mainFeatures.push_back( { "Shader Image Gather Extended", bool(device.features.shaderImageGatherExtended) } );
        d.mainFeatures.push_back( { "Shader Storage Image Extended Formats", bool(device.features.shaderStorageImageExtendedFormats) } );
        d.mainFeatures.push_back( { "Shader Storage Image Multisample", bool(device.features.shaderStorageImageMultisample) } );
        d.mainFeatures.push_back( { "Shader Storage Image Read Without Format", bool(device.features.shaderStorageImageReadWithoutFormat) } );
        d.mainFeatures.push_back( { "Shader Storage image Write Without Format", bool(device.features.shaderStorageImageWriteWithoutFormat) } );
        d.mainFeatures.push_back( { "Shader Uniform Buffer Array Dynamic Indexing", bool(device.features.shaderUniformBufferArrayDynamicIndexing) } );
        d.mainFeatures.push_back( { "Shader Sampled Array Dynamic Indexing", bool(device.features.shaderSampledImageArrayDynamicIndexing) } );
        d.mainFeatures.push_back( { "Shader Storage Buffer Array Dynamic Indexing", bool(device.features.shaderStorageBufferArrayDynamicIndexing) } );
        d.mainFeatures.push_back( { "Shader Storage Image Array Dynamic Indexing", bool(device.features.shaderStorageImageArrayDynamicIndexing) } );
        d.mainFeatures.push_back( { "Shader Clip Distance", bool(device.features.shaderClipDistance) } );
        d.mainFeatures.push_back( { "Shader Cull Distance", bool(device.features.shaderCullDistance) } );
        d.mainFeatures.push_back( { "Shader Float64", bool(device.features.shaderFloat64) } );
        d.mainFeatures.push_back( { "Shader Int64", bool(device.features.shaderInt64) } );
        d.mainFeatures.push_back( { "Shader Int16", bool(device.features.shaderInt16) } );
        d.mainFeatures.push_back( { "Shader Resource Residency", bool(device.features.shaderResourceResidency) } );
        d.mainFeatures.push_back( { "Shader Resource Min LOD", bool(device.features.shaderResourceMinLod) } );
        d.mainFeatures.push_back( { "Sparse Binding", bool(device.features.sparseBinding) } );
        d.mainFeatures.push_back( { "Sparse Residency Buffer", bool(device.features.sparseResidencyBuffer) } );
        d.mainFeatures.push_back( { "Sparse Residency Image 2D", bool(device.features.sparseResidencyImage2D) } );
        d.mainFeatures.push_back( { "Sparse Residency Image 3D", bool(device.features.sparseResidencyImage3D) } );
        d.mainFeatures.push_back( { "Sparse Residency 2 Samples", bool(device.features.sparseResidency2Samples) } );
        d.mainFeatures.push_back( { "Sparse Residency 4 Samples", bool(device.features.sparseResidency4Samples) } );
        d.mainFeatures.push_back( { "Sparse Residency 8 Samples", bool(device.features.sparseResidency8Samples) } );
        d.mainFeatures.push_back( { "Sparse Residency 16 Samples", bool(device.features.sparseResidency16Samples) } );
        d.mainFeatures.push_back( { "Sparse Residency Aliased", bool(device.features.sparseResidencyAliased) } );
        d.mainFeatures.push_back( { "Variable Multisample Rate", bool(device.features.variableMultisampleRate) } );
        d.mainFeatures.push_back( { "Inherited Queries", bool(device.features.inheritedQueries) } );

        for (const VkExtensionProperties& ex : device.extensions)
        {
            d.extensions.push_back(
                std::make_pair(ex.extensionName,
                               std::to_string(ex.specVersion)));
        }

        const QStringList descs = readDescription("://descriptions/limits.txt");
        int descIndex = 0;
        d.limits.push_back(
            { "Max Image Dimension 1D",
              std::to_string(device.properties.limits.maxImageDimension1D),
              descs[descIndex++].toStdString() });

        d.limits.push_back(
            { "Max Image Dimension 2D",
              std::to_string(device.properties.limits.maxImageDimension2D),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Image Dimension 3D",
              std::to_string(device.properties.limits.maxImageDimension3D),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Image Dimension Cube",
              std::to_string(device.properties.limits.maxImageDimensionCube),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Image Array Layers",
              std::to_string(device.properties.limits.maxImageArrayLayers),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Texel Buffer Elements",
              std::to_string(device.properties.limits.maxTexelBufferElements),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Uniform BufferRange",
              std::to_string(device.properties.limits.maxUniformBufferRange),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Storage Buffer Range",
              std::to_string(device.properties.limits.maxStorageBufferRange),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Push Constants Size",
              std::to_string(device.properties.limits.maxPushConstantsSize),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Memory AllocationCount",
              std::to_string(device.properties.limits.maxMemoryAllocationCount),
              descs[descIndex++].toStdString() } );
        d.limits.push_back(
            { "Max Sampler Allocation Count",
              std::to_string(device.properties.limits.maxSamplerAllocationCount),
              descs[descIndex++].toStdString() } );

        d.limits.push_back( { "bufferImageGranularity", std::to_string(device.properties.limits.bufferImageGranularity), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "sparseAddressSpaceSize", std::to_string(device.properties.limits.sparseAddressSpaceSize), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxBoundDescriptorSets", std::to_string(device.properties.limits.maxBoundDescriptorSets), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxPerStageDescriptorSamplers", std::to_string(device.properties.limits.maxPerStageDescriptorSamplers), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxPerStageDescriptorUniformBuffers", std::to_string(device.properties.limits.maxPerStageDescriptorUniformBuffers), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxPerStageDescriptorStorageBuffers", std::to_string(device.properties.limits.maxPerStageDescriptorStorageBuffers), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxPerStageDescriptorSampledImages", std::to_string(device.properties.limits.maxPerStageDescriptorSampledImages), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxPerStageDescriptorStorageImages", std::to_string(device.properties.limits.maxPerStageDescriptorStorageImages), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxPerStageDescriptorInputAttachments", std::to_string(device.properties.limits.maxPerStageDescriptorInputAttachments), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxPerStageResources", std::to_string(device.properties.limits.maxPerStageResources), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetSamplers", std::to_string(device.properties.limits.maxDescriptorSetSamplers), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetUniformBuffers", std::to_string(device.properties.limits.maxDescriptorSetUniformBuffers), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetUniformBuffersDynamic", std::to_string(device.properties.limits.maxDescriptorSetUniformBuffersDynamic), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetStorageBuffers", std::to_string(device.properties.limits.maxDescriptorSetStorageBuffers), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetStorageBuffersDynamic", std::to_string(device.properties.limits.maxDescriptorSetStorageBuffersDynamic), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetSampledImages", std::to_string(device.properties.limits.maxDescriptorSetSampledImages), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetStorageImages", std::to_string(device.properties.limits.maxDescriptorSetStorageImages), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxDescriptorSetInputAttachments", std::to_string(device.properties.limits.maxDescriptorSetInputAttachments), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxVertexInputAttributes", std::to_string(device.properties.limits.maxVertexInputAttributes), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxVertexInputBindings", std::to_string(device.properties.limits.maxVertexInputBindings), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxVertexInputAttributeOffset", std::to_string(device.properties.limits.maxVertexInputAttributeOffset), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxVertexInputBindingStride", std::to_string(device.properties.limits.maxVertexInputBindingStride), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxVertexOutputComponents", std::to_string(device.properties.limits.maxVertexOutputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationGenerationLevel", std::to_string(device.properties.limits.maxTessellationGenerationLevel), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationPatchSize", std::to_string(device.properties.limits.maxTessellationPatchSize), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationControlPerVertexInputComponents", std::to_string(device.properties.limits.maxTessellationControlPerVertexInputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationControlPerVertexOutputComponents", std::to_string(device.properties.limits.maxTessellationControlPerVertexOutputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationControlPerPatchOutputComponents", std::to_string(device.properties.limits.maxTessellationControlPerPatchOutputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationControlTotalOutputComponents", std::to_string(device.properties.limits.maxTessellationControlTotalOutputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationEvaluationInputComponents", std::to_string(device.properties.limits.maxTessellationEvaluationInputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTessellationEvaluationOutputComponents", std::to_string(device.properties.limits.maxTessellationEvaluationOutputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxGeometryShaderInvocations", std::to_string(device.properties.limits.maxGeometryShaderInvocations), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxGeometryInputComponents", std::to_string(device.properties.limits.maxGeometryInputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxGeometryOutputComponents", std::to_string(device.properties.limits.maxGeometryOutputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxGeometryOutputVertices", std::to_string(device.properties.limits.maxGeometryOutputVertices), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxGeometryTotalOutputComponents", std::to_string(device.properties.limits.maxGeometryTotalOutputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxFragmentInputComponents", std::to_string(device.properties.limits.maxFragmentInputComponents), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxFragmentOutputAttachments", std::to_string(device.properties.limits.maxFragmentOutputAttachments), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxFragmentDualSrcAttachments", std::to_string(device.properties.limits.maxFragmentDualSrcAttachments), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxFragmentCombinedOutputResources", std::to_string(device.properties.limits.maxFragmentCombinedOutputResources), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxComputeSharedMemorySize", std::to_string(device.properties.limits.maxComputeSharedMemorySize), descs[descIndex++].toStdString() } );


        std::stringstream ss;
        ss << "x: " << std::to_string(device.properties.limits.maxComputeWorkGroupCount[0]) << ", ";
        ss << "y: " << std::to_string(device.properties.limits.maxComputeWorkGroupCount[1]) << ", ";
        ss << "z: " << std::to_string(device.properties.limits.maxComputeWorkGroupCount[2]);

        d.limits.push_back( { "maxComputeWorkGroupCount", ss.str(), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxComputeWorkGroupInvocations", std::to_string(device.properties.limits.maxComputeWorkGroupInvocations), descs[descIndex++].toStdString() } );

        std::stringstream maxComputeWorkGroupSizeStr;
        maxComputeWorkGroupSizeStr << "x: " << std::to_string(device.properties.limits.maxComputeWorkGroupSize[0]) << ", ";
        maxComputeWorkGroupSizeStr << "y: " << std::to_string(device.properties.limits.maxComputeWorkGroupSize[1]) << ", ";
        maxComputeWorkGroupSizeStr << "z: " << std::to_string(device.properties.limits.maxComputeWorkGroupSize[2]);

        d.limits.push_back( { "Max Compute Work Group Size",
                              maxComputeWorkGroupSizeStr.str(),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Sub Pixel Precision Bits",
                              std::to_string(device.properties.limits.subPixelPrecisionBits),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Sub Texel Precision Bits",
                              std::to_string(device.properties.limits.subTexelPrecisionBits),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Mipmap Precision Bits",
                              std::to_string(device.properties.limits.mipmapPrecisionBits),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Max Draw Indexed Index Value",
                              std::to_string(device.properties.limits.maxDrawIndexedIndexValue),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Max Draw Indirect Count",
                              std::to_string(device.properties.limits.maxDrawIndirectCount),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Max Sampler LodBias",
                              std::to_string(device.properties.limits.maxSamplerLodBias),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Max Sampler Anisotropy",
                              std::to_string(device.properties.limits.maxSamplerAnisotropy),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Max Viewports",
                              std::to_string(device.properties.limits.maxViewports),
                              descs[descIndex++].toStdString() } );

        std::stringstream maxViewportStr;
        maxViewportStr << "x: " << device.properties.limits.maxViewportDimensions[0] << ", "
                       << "y: " << device.properties.limits.maxViewportDimensions[1];
        d.limits.push_back(
            { "Max Viewport Dimensions",
              maxViewportStr.str(),
              descs[descIndex++].toStdString() } );

        std::stringstream viewportBoundsRange;
        viewportBoundsRange << "[ " << device.properties.limits.viewportBoundsRange[0] << ", "
                                    << device.properties.limits.viewportBoundsRange[1] << "]";

        d.limits.push_back( { "Viewport Bounds Range",
                              viewportBoundsRange.str(),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "viewportSubPixelBits", std::to_string(device.properties.limits.viewportSubPixelBits), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "minMemoryMapAlignment", std::to_string(device.properties.limits.minMemoryMapAlignment), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "minTexelBufferOffsetAlignment", std::to_string(device.properties.limits.minTexelBufferOffsetAlignment), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "minUniformBufferOffsetAlignment", std::to_string(device.properties.limits.minUniformBufferOffsetAlignment), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "minStorageBufferOffsetAlignment", std::to_string(device.properties.limits.minStorageBufferOffsetAlignment), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "minTexelOffset", std::to_string(device.properties.limits.minTexelOffset), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTexelOffset", std::to_string(device.properties.limits.maxTexelOffset), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "minTexelGatherOffset", std::to_string(device.properties.limits.minTexelGatherOffset), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxTexelGatherOffset", std::to_string(device.properties.limits.maxTexelGatherOffset), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "minInterpolationOffset", std::to_string(device.properties.limits.minInterpolationOffset), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxInterpolationOffset", std::to_string(device.properties.limits.maxInterpolationOffset), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "subPixelInterpolationOffsetBits", std::to_string(device.properties.limits.subPixelInterpolationOffsetBits), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxFramebufferWidth", std::to_string(device.properties.limits.maxFramebufferWidth), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxFramebufferHeight", std::to_string(device.properties.limits.maxFramebufferHeight), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxFramebufferLayers", std::to_string(device.properties.limits.maxFramebufferLayers), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "framebufferColorSampleCounts", std::to_string(device.properties.limits.framebufferColorSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "framebufferDepthSampleCounts", std::to_string(device.properties.limits.framebufferDepthSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "framebufferStencilSampleCounts", std::to_string(device.properties.limits.framebufferStencilSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "framebufferNoAttachmentsSampleCounts", std::to_string(device.properties.limits.framebufferNoAttachmentsSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxColorAttachments", std::to_string(device.properties.limits.maxColorAttachments), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "sampledImageColorSampleCounts", std::to_string(device.properties.limits.sampledImageColorSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "sampledImageIntegerSampleCounts", std::to_string(device.properties.limits.sampledImageIntegerSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "sampledImageDepthSampleCounts", std::to_string(device.properties.limits.sampledImageDepthSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "sampledImageStencilSampleCounts", std::to_string(device.properties.limits.sampledImageStencilSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "storageImageSampleCounts", std::to_string(device.properties.limits.storageImageSampleCounts), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxSampleMaskWords", std::to_string(device.properties.limits.maxSampleMaskWords), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "timestampComputeAndGraphics", std::to_string(device.properties.limits.timestampComputeAndGraphics), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "timestampPeriod", std::to_string(device.properties.limits.timestampPeriod), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxClipDistances", std::to_string(device.properties.limits.maxClipDistances), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxCullDistances", std::to_string(device.properties.limits.maxCullDistances), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "maxCombinedClipAndCullDistances", std::to_string(device.properties.limits.maxCombinedClipAndCullDistances), descs[descIndex++].toStdString() } );
        d.limits.push_back( { "discreteQueuePriorities", std::to_string(device.properties.limits.discreteQueuePriorities), descs[descIndex++].toStdString() } );

        std::stringstream pointSizeRangeStr;
        pointSizeRangeStr << "x: " << device.properties.limits.pointSizeRange[0] << ", "
                          << "y: " << device.properties.limits.pointSizeRange[1];

        d.limits.push_back( { "Point Size Range",
                              pointSizeRangeStr.str(),
                              descs[descIndex++].toStdString() } );

        std::stringstream lineWidthRangeStr;
        lineWidthRangeStr << "x: " << device.properties.limits.lineWidthRange[0] << ", "
                          << "y: " << device.properties.limits.lineWidthRange[1];

        d.limits.push_back( { "Line Width Range",
                              lineWidthRangeStr.str(),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Point Size Granularity",
                              std::to_string(device.properties.limits.pointSizeGranularity),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Line Width Granularity",
                              std::to_string(device.properties.limits.lineWidthGranularity),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Strict Lines",
                              std::to_string(device.properties.limits.strictLines),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Standard Sample Locations",
                              std::to_string(device.properties.limits.standardSampleLocations),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Optimal Buffer Copy Offset Alignment",
                              std::to_string(device.properties.limits.optimalBufferCopyOffsetAlignment),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Optimal Buffer Copy Row Pitch Alignment",
                              std::to_string(device.properties.limits.optimalBufferCopyRowPitchAlignment),
                              descs[descIndex++].toStdString() } );
        d.limits.push_back( { "Non-Coherent Atom Size",
                              std::to_string(device.properties.limits.nonCoherentAtomSize),
                              descs[descIndex++].toStdString() } );

        out->physicalDeviceData.push_back(d);
    }

    return out;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct Controller::Impl
{
    // Main window
    std::unique_ptr<MainWindow> mainWindow;
    // Vulkan objects
    std::shared_ptr<VulkanObjects> vulkanObjects;
    // Capabilities data created from vulkan objects.
    std::shared_ptr<Data> capabilitiesData;
};

/* -------------------------------------------------------------------------- */

Controller::Controller()
    : impl(std::make_shared<Impl>())
{
    impl->vulkanObjects = std::make_shared<VulkanObjects>();
    impl->capabilitiesData = createCapabilitiesData(impl->vulkanObjects);
}

/* -------------------------------------------------------------------------- */

void Controller::showUi()
{
    impl->mainWindow = std::unique_ptr<MainWindow>(new MainWindow());
    impl->mainWindow->setData(impl->capabilitiesData);
    impl->mainWindow->show();
}

} // namespace vk_capabilities
} // namespace kuu

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

std::vector<std::pair<std::string, std::string>> readDescription(const QString& filepath)
{
    // Transforms a variable name from 'myVeryOwnVariable' to
    // 'My Very Own Variable'.
    auto transformVariable = [](const QString& in)
    {
        QString out;
        for (int i = 0; i < in.size(); ++i)
        {
            QChar c = in[i];
            if (c.isUpper())
                out.append(" ");
            if (i == 0)
                c = c.toUpper();
            out.append(c);
        }
        return out;
    };

    // Splits a long description text into smaller lines.
    auto splitToLines = [](const QString& in, const int wordMax = 10)
    {
        QStringList inWords = in.split(" ");
        QStringList outWords;
        QString out;
        int wordCount = 0;
        for (int i = 0; i < inWords.size(); ++i)
        {
            if (wordCount < wordMax)
            {
                outWords << inWords[i];
                wordCount++;
            }
            else
            {
                wordCount = 0;
                out += outWords.join(" ") += "\n";
                outWords.clear();
            }
        }
        out += outWords.join(" ");
        return out;
    };

    QFile qssFile(filepath);
    if (!qssFile.open(QIODevice::ReadOnly))
    {
        std::cerr << __FUNCTION__
                  << "Failed to read desciptions from "
                  << filepath.toStdString()
                  << std::endl;
        return std::vector<std::pair<std::string, std::string>>();
    }

    std::vector<std::pair<std::string, std::string>> out;
    QTextStream ts(&qssFile);
    while(!ts.atEnd())
    {
        QString line = ts.readLine().trimmed();
        if (line.startsWith(";"))
            continue;
        if (line.isEmpty())
            continue;

        QString variable = line.section(" ", 0, 0);
        variable = transformVariable(variable);

        QString desc = variable + " " + line.section(" ", 1);
        desc = splitToLines(desc);

        out.push_back(std::make_pair(
            variable.toStdString(),
            desc.toStdString()));
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

        const VkPhysicalDeviceLimits& limits = device.properties.limits;
        const std::vector<std::pair<std::string, std::string>> descs =
            readDescription("://descriptions/limits.txt");
        int descIndex = 0;

        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxImageDimension1D), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxImageDimension2D), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxImageDimension3D), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxImageDimensionCube), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxImageArrayLayers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTexelBufferElements), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxUniformBufferRange), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxStorageBufferRange), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPushConstantsSize), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxMemoryAllocationCount), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxSamplerAllocationCount), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.bufferImageGranularity), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.sparseAddressSpaceSize), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxBoundDescriptorSets), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorSamplers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorUniformBuffers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorStorageBuffers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorSampledImages), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorStorageImages), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorInputAttachments), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxPerStageResources), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetSamplers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetUniformBuffers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetUniformBuffersDynamic), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetStorageBuffers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetStorageBuffersDynamic), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetSampledImages), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetStorageImages), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDescriptorSetInputAttachments), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxVertexInputAttributes), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxVertexInputBindings), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxVertexInputAttributeOffset), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxVertexInputBindingStride), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxVertexOutputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationGenerationLevel), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationPatchSize), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationControlPerVertexInputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationControlPerVertexOutputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationControlPerPatchOutputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationControlTotalOutputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationEvaluationInputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTessellationEvaluationOutputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxGeometryShaderInvocations), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxGeometryInputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxGeometryOutputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxGeometryOutputVertices), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxGeometryTotalOutputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxFragmentInputComponents), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxFragmentOutputAttachments), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxFragmentDualSrcAttachments), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxFragmentCombinedOutputResources), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxComputeSharedMemorySize), descs[descIndex++].second });

        std::stringstream ss;
        ss << "x: " << std::to_string(limits.maxComputeWorkGroupCount[0]) << ", ";
        ss << "y: " << std::to_string(limits.maxComputeWorkGroupCount[1]) << ", ";
        ss << "z: " << std::to_string(limits.maxComputeWorkGroupCount[2]);

        d.limits.push_back( { descs[descIndex].first, ss.str(), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxComputeWorkGroupInvocations), descs[descIndex++].second });

        std::stringstream maxComputeWorkGroupSizeStr;
        maxComputeWorkGroupSizeStr << "x: " << std::to_string(limits.maxComputeWorkGroupSize[0]) << ", ";
        maxComputeWorkGroupSizeStr << "y: " << std::to_string(limits.maxComputeWorkGroupSize[1]) << ", ";
        maxComputeWorkGroupSizeStr << "z: " << std::to_string(limits.maxComputeWorkGroupSize[2]);

        d.limits.push_back( { descs[descIndex].first, maxComputeWorkGroupSizeStr.str(), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.subPixelPrecisionBits), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.subTexelPrecisionBits), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.mipmapPrecisionBits), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDrawIndexedIndexValue), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxDrawIndirectCount), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxSamplerLodBias), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxSamplerAnisotropy), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxViewports), descs[descIndex++].second });

        std::stringstream maxViewportStr;
        maxViewportStr << "x: " << limits.maxViewportDimensions[0] << ", "
                       << "y: " << limits.maxViewportDimensions[1];
        d.limits.push_back(
            { descs[descIndex].first,
              maxViewportStr.str(),
              descs[descIndex++].second });

        std::stringstream viewportBoundsRange;
        viewportBoundsRange << "[ " << limits.viewportBoundsRange[0] << ", "
                                    << limits.viewportBoundsRange[1] << "]";

        d.limits.push_back( { descs[descIndex].first, viewportBoundsRange.str(), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.viewportSubPixelBits), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.minMemoryMapAlignment), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.minTexelBufferOffsetAlignment), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.minUniformBufferOffsetAlignment), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.minStorageBufferOffsetAlignment), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.minTexelOffset), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTexelOffset), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.minTexelGatherOffset), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxTexelGatherOffset), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.minInterpolationOffset), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxInterpolationOffset), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.subPixelInterpolationOffsetBits), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxFramebufferWidth), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxFramebufferHeight), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxFramebufferLayers), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.framebufferColorSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.framebufferDepthSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.framebufferStencilSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.framebufferNoAttachmentsSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxColorAttachments), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.sampledImageColorSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.sampledImageIntegerSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.sampledImageDepthSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.sampledImageStencilSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.storageImageSampleCounts), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxSampleMaskWords), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.timestampComputeAndGraphics), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.timestampPeriod), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxClipDistances), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxCullDistances), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.maxCombinedClipAndCullDistances), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.discreteQueuePriorities), descs[descIndex++].second });

        std::stringstream pointSizeRangeStr;
        pointSizeRangeStr << "x: " << limits.pointSizeRange[0] << ", "
                          << "y: " << limits.pointSizeRange[1];

        d.limits.push_back( { descs[descIndex].first, pointSizeRangeStr.str(), descs[descIndex++].second });

        std::stringstream lineWidthRangeStr;
        lineWidthRangeStr << "x: " << limits.lineWidthRange[0] << ", "
                          << "y: " << limits.lineWidthRange[1];

        d.limits.push_back( { descs[descIndex].first, lineWidthRangeStr.str(),                                   descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.pointSizeGranularity),               descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.lineWidthGranularity),               descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.strictLines),                        descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.standardSampleLocations),            descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.optimalBufferCopyOffsetAlignment),   descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.optimalBufferCopyRowPitchAlignment), descs[descIndex++].second });
        d.limits.push_back( { descs[descIndex].first, std::to_string(limits.nonCoherentAtomSize),                descs[descIndex++].second });

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

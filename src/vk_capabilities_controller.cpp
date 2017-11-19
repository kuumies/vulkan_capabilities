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

std::vector<std::pair<std::string, std::string>> readFormat(const QString& filepath)
{
    // Transforms a variable name from 'VK_FORMAT_B5G5R5A1_UNORM_PACK16' to
    // 'B5G5R5A1 UNORM PACK16'.
    auto transformVariable = [](QString in)
    {
//        in = in.remove("VK_FORMAT_");
//        in = in.replace("_", " ");
        return in;
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
                  << "Failed to read formats from "
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

/* ---------------------------------------------------------------- */

std::vector<std::pair<std::string, std::string>> readDescription(const QString& filepath)
{
    // Transforms a variable name from 'myVeryOwnVariable' to
    // 'My Very Own Variable'.
    // BUG: issues with array indices
    auto transformVariable = [](const QString& in)
    {
        QString out;
        for (int i = 0; i < in.size(); ++i)
        {
            QChar c = in[i];
            if (c.isUpper() || c.isDigit())
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
       Defines a format struct.
     * ---------------------------------------------------------------------- */
    struct Format
    {
        VkFormat format;
        VkFormatProperties properties;
    };

    /* ---------------------------------------------------------------------- *
       Defines a physical device data struct.
     * ---------------------------------------------------------------------- */
    struct PhysicalDevice
    {
        struct Queue
        {
            const uint32_t queueFamilyIndex;
            const VkQueueFamilyProperties properties;
            const VkBool32 presentationSupport;
        };

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
        const std::vector<Queue> queues;                                            // Queues
        const VkPhysicalDeviceMemoryProperties memoryProperties;                    // Memory properties
        const std::vector<Format> formats;                                          // Formats
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

            std::vector<PhysicalDevice::Queue> queues;
            for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyPropertyCount; ++queueFamilyIndex)
            {
                const VkQueueFamilyProperties& property = queueFamilyProperties[queueFamilyIndex];

                queues.push_back(
                {
                    queueFamilyIndex,
                    property,
                    VK_FALSE
                });
            }

            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(
                physicalDevice,
                &memoryProperties);


            std::vector<Format> formats;
            for (int f = VK_FORMAT_R4G4_UNORM_PACK8; f <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; ++f)
            {
                VkFormat fmt = VkFormat(f);
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(
                    physicalDevice,
                    fmt,
                    &props);
                formats.push_back( { fmt, props} );
            }

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
                    devExtensions,
                    queues,
                    memoryProperties,
                    formats
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

    for (const VulkanObjects::PhysicalDevice& device : vulkanObjects->physicalDevices)
    {
        Data::PhysicalDeviceData d;
        d.name = device.properties.deviceName;

        auto addRow = [&](
                std::vector<Data::Row>& rows,
                const std::string& name,
                const std::string& value)
        {
            rows.push_back(
            {{
                { Data::Cell::Style::NameLabel,  name,  "", -1 },
                { Data::Cell::Style::ValueLabel, value, "", -1 },
            }});
        };

        std::vector<Data::Row> propertiesRows;
        addRow(propertiesRows, "Name",                device.properties.deviceName);
        addRow(propertiesRows, "Type",                toString(device.properties.deviceType));
        addRow(propertiesRows, "API Version",         versionNumber(device.properties.apiVersion));
        addRow(propertiesRows, "Driver Version",      versionNumber(device.properties.driverVersion));
        addRow(propertiesRows, "Vendor ID",           hexValueToString(device.properties.vendorID));
        addRow(propertiesRows, "Device ID",           hexValueToString(device.properties.deviceID));
        addRow(propertiesRows, "Pipeline Cache UUID", toString(device.properties.pipelineCacheUUID));

        d.properties.resize(1);
        d.properties[0].valueRows = propertiesRows;
        d.properties[0].header.cells.push_back( { Data::Cell::Style::Header, "Property",  "", -1  } );
        d.properties[0].header.cells.push_back( { Data::Cell::Style::Header, "Value",     "", -1  } );

        std::vector<Data::Row> instanceExtensionsRows;
        for (const VkExtensionProperties& ex : vulkanObjects->instanceExtensions)
            addRow(instanceExtensionsRows,
                   ex.extensionName,
                   std::to_string(ex.specVersion));

        std::vector<Data::Row> deviceExtensionsRows;
        for (const VkExtensionProperties& ex : device.extensions)
            addRow(deviceExtensionsRows, ex.extensionName, std::to_string(ex.specVersion));

        d.extensions.resize(2);
        d.extensions[0].valueRows = instanceExtensionsRows;
        d.extensions[0].header.cells.push_back( { Data::Cell::Style::Header, "Supported Instance Extension", "", -1  } );
        d.extensions[0].header.cells.push_back( { Data::Cell::Style::Header, "Version",                      "", -1  } );
        d.extensions[1].valueRows = deviceExtensionsRows;
        d.extensions[1].header.cells.push_back( { Data::Cell::Style::Header, "Supported Device Extension", "", -1  } );
        d.extensions[1].header.cells.push_back( { Data::Cell::Style::Header, "Version",                    "", -1  } );


        auto addLayerRow = [&](
                std::vector<Data::Row>& rows,
                const std::string& name,
                const std::string& desc,
                const std::string& specVersion,
                const std::string& implVersion)
        {
            rows.push_back(
            {{
                { Data::Cell::Style::NameLabel,  name,        desc, -1 },
                { Data::Cell::Style::ValueLabel, specVersion, "",   -1 },
                { Data::Cell::Style::ValueLabel, implVersion, "",   -1 },
            }});
        };

        std::vector<Data::Row> instanceLayerRows;
        for (const VkLayerProperties& l : vulkanObjects->instanceLayers)
            addLayerRow(instanceLayerRows,
                        l.layerName,
                        l.description,
                        vk::stringify::versionNumber(l.specVersion),
                        std::to_string(l.implementationVersion));

        d.layers.resize(1);
        d.layers[0].valueRows = instanceLayerRows;
        d.layers[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",          "", -1  } );
        d.layers[0].header.cells.push_back( { Data::Cell::Style::Header, "Spec. Version", "", -1  } );
        d.layers[0].header.cells.push_back( { Data::Cell::Style::Header, "Impl. Version", "", -1  } );

        auto vkboolToStr = [](const VkBool32& b)
        { return  b == VK_TRUE ? "Supported" : "Unsupported"; };
        auto vkboolToStyle = [](const VkBool32& b)
        { return  b == VK_TRUE ? Data::Cell::Style::ValueLabelValid
                               : Data::Cell::Style::ValueLabelInvalid; };

        std::vector<Data::Row> featureRows;
        auto addFeatureRow = [&](
                const std::string& name,
                const VkBool32& b)
        {
            featureRows.push_back(
            {{
                { Data::Cell::Style::NameLabel, name,           "", -1 },
                { vkboolToStyle(b),             vkboolToStr(b), "", -1 },
            }});
        };

        const VkPhysicalDeviceFeatures f = device.features;
        addFeatureRow("Robust Buffer Access",   f.robustBufferAccess);
        addFeatureRow("Full Draw Index Uint32", f.fullDrawIndexUint32);
        addFeatureRow("Image Cube Array",       f.imageCubeArray);
        addFeatureRow("Independent Blend",      f.independentBlend);
        addFeatureRow("Geometry Shader",        f.geometryShader);
        addFeatureRow("Tesselation Shader",     f.tessellationShader);
        addFeatureRow("Sample Rate Shading",    f.sampleRateShading);
        addFeatureRow("Dual SRC Blend",         f.dualSrcBlend);
        addFeatureRow("Logic OP",               f.logicOp);
        addFeatureRow("Multi Draw Indirect",    f.multiDrawIndirect);
        addFeatureRow("Draw Inderect First Instance", f.drawIndirectFirstInstance);
        addFeatureRow("Depth CLamp",            f.depthClamp);
        addFeatureRow("Depth Bias Clamp",       f.depthBiasClamp);
        addFeatureRow("Fill Mode Non Solid",    f.fillModeNonSolid);
        addFeatureRow("Depth Bounds",           f.depthBounds);
        addFeatureRow("Wide Lines",             f.wideLines);
        addFeatureRow("Large Points",           f.largePoints);
        addFeatureRow("Alpha To One",           f.alphaToOne);
        addFeatureRow("Multi Viewport",         f.multiViewport);
        addFeatureRow("Sampler Anisotropy",     f.samplerAnisotropy);
        addFeatureRow("Texture Compression ETC2",                     f.textureCompressionETC2);
        addFeatureRow("Texture Compression ASTC_LDR",                 f.textureCompressionASTC_LDR);
        addFeatureRow("Texture Compression BC",                       f.textureCompressionBC);
        addFeatureRow("Occlusion Query Precise",                      f.occlusionQueryPrecise);
        addFeatureRow("Pipeline Staticstics Query",                   f.pipelineStatisticsQuery);
        addFeatureRow("Vertex Pipeline Stores and Atomics",           f.vertexPipelineStoresAndAtomics);
        addFeatureRow("Fragment Stores and Atomics",                  f.fragmentStoresAndAtomics);
        addFeatureRow("Shader Tesselation And Geometry Point Size",   f.shaderTessellationAndGeometryPointSize);
        addFeatureRow("Shader Image Gather Extended",                 f.shaderImageGatherExtended);
        addFeatureRow("Shader Storage Image Extended Formats",        f.shaderStorageImageExtendedFormats);
        addFeatureRow("Shader Storage Image Multisample",             f.shaderStorageImageMultisample);
        addFeatureRow("Shader Storage Image Read Without Format",     f.shaderStorageImageReadWithoutFormat);
        addFeatureRow("Shader Storage image Write Without Format",    f.shaderStorageImageWriteWithoutFormat);
        addFeatureRow("Shader Uniform Buffer Array Dynamic Indexing", f.shaderUniformBufferArrayDynamicIndexing);
        addFeatureRow("Shader Sampled Array Dynamic Indexing",        f.shaderSampledImageArrayDynamicIndexing);
        addFeatureRow("Shader Storage Buffer Array Dynamic Indexing", f.shaderStorageBufferArrayDynamicIndexing);
        addFeatureRow("Shader Storage Image Array Dynamic Indexing",  f.shaderStorageImageArrayDynamicIndexing);
        addFeatureRow("Shader Clip Distance",        f.shaderClipDistance);
        addFeatureRow("Shader Cull Distance",        f.shaderCullDistance);
        addFeatureRow("Shader Float64",              f.shaderFloat64);
        addFeatureRow("Shader Int64",                f.shaderInt64);
        addFeatureRow("Shader Int16",                f.shaderInt16);
        addFeatureRow("Shader Resource Residency",   f.shaderResourceResidency);
        addFeatureRow("Shader Resource Min LOD",     f.shaderResourceMinLod);
        addFeatureRow("Sparse Binding",              f.sparseBinding);
        addFeatureRow("Sparse Residency Buffer",     f.sparseResidencyBuffer);
        addFeatureRow("Sparse Residency Image 2D",   f.sparseResidencyImage2D);
        addFeatureRow("Sparse Residency Image 3D",   f.sparseResidencyImage3D);
        addFeatureRow("Sparse Residency 2 Samples",  f.sparseResidency2Samples);
        addFeatureRow("Sparse Residency 4 Samples",  f.sparseResidency4Samples);
        addFeatureRow("Sparse Residency 8 Samples",  f.sparseResidency8Samples);
        addFeatureRow("Sparse Residency 16 Samples", f.sparseResidency16Samples);
        addFeatureRow("Sparse Residency Aliased",    f.sparseResidencyAliased);
        addFeatureRow("Variable Multisample Rate",   f.variableMultisampleRate);
        addFeatureRow("Inherited Queries",           f.inheritedQueries);

        d.features.resize(1);
        d.features[0].valueRows = featureRows;
        d.features[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",           "", -1  } );
        d.features[0].header.cells.push_back( { Data::Cell::Style::Header, "Support Status", "", -1  } );

        auto pointToStr = [](const uint32_t* p, int count)
        {
            std::stringstream ss;
            if (count > 1)
            {
                ss << "x: " << std::to_string(p[0]) << ", ";
                ss << "y: " << std::to_string(p[1]) << ", ";
            }
            if (count > 2)
            {
                ss << "z: " << std::to_string(p[2]);
            }
            return ss.str();
        };

        auto pointfToStr = [](const float* p, int count)
        {
            std::stringstream ss;
            if (count > 1)
            {
                ss << "x: " << std::to_string(p[0]) << ", ";
                ss << "y: " << std::to_string(p[1]) << ", ";
            }
            if (count > 2)
            {
                ss << "z: " << std::to_string(p[2]);
            }
            return ss.str();
        };

        auto rangeToStr = [](const float* p, int count)
        {
            std::stringstream ss;
            ss << "[";

            if (count > 0)
                ss << p[0];
            if (count > 1)
                ss << ", " << p[1];

            ss << "]";
            return ss.str();
        };

        const VkPhysicalDeviceLimits& limits = device.properties.limits;
        const std::vector<std::pair<std::string, std::string>> descs =
            readDescription("://descriptions/limits.txt");
        int descIndex = 0;

        std::vector<Data::Row> limitRows;
        auto addLimitRow = [&](
                const std::string& name,
                const std::string& limit,
                const std::string& desc
                )
        {
            limitRows.push_back(
            {{
                { Data::Cell::Style::NameLabel, name,  desc, -1 },
                { Data::Cell::Style::ValueLabel,limit, "",   -1 },
            }});
            descIndex++;
        };

        addLimitRow(descs[descIndex].first, std::to_string(limits.maxImageDimension1D), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxImageDimension2D), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxImageDimension3D), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxImageDimensionCube), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxImageArrayLayers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTexelBufferElements), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxUniformBufferRange), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxStorageBufferRange), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPushConstantsSize), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxMemoryAllocationCount), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxSamplerAllocationCount), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.bufferImageGranularity), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.sparseAddressSpaceSize), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxBoundDescriptorSets), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorSamplers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorUniformBuffers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorStorageBuffers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorSampledImages), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorStorageImages), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPerStageDescriptorInputAttachments), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxPerStageResources), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetSamplers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetUniformBuffers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetUniformBuffersDynamic), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetStorageBuffers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetStorageBuffersDynamic), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetSampledImages), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetStorageImages), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDescriptorSetInputAttachments), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxVertexInputAttributes), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxVertexInputBindings), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxVertexInputAttributeOffset), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxVertexInputBindingStride), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxVertexOutputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationGenerationLevel), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationPatchSize), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationControlPerVertexInputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationControlPerVertexOutputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationControlPerPatchOutputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationControlTotalOutputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationEvaluationInputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTessellationEvaluationOutputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxGeometryShaderInvocations), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxGeometryInputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxGeometryOutputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxGeometryOutputVertices), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxGeometryTotalOutputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxFragmentInputComponents), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxFragmentOutputAttachments), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxFragmentDualSrcAttachments), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxFragmentCombinedOutputResources), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxComputeSharedMemorySize), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, pointToStr(limits.maxComputeWorkGroupCount, 3), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxComputeWorkGroupInvocations), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, pointToStr(limits.maxComputeWorkGroupSize, 3), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.subPixelPrecisionBits), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.subTexelPrecisionBits), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.mipmapPrecisionBits), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDrawIndexedIndexValue), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxDrawIndirectCount), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxSamplerLodBias), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxSamplerAnisotropy), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxViewports), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, pointToStr(limits.maxViewportDimensions, 2), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, rangeToStr(limits.viewportBoundsRange, 2), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.viewportSubPixelBits), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.minMemoryMapAlignment), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.minTexelBufferOffsetAlignment), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.minUniformBufferOffsetAlignment), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.minStorageBufferOffsetAlignment), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.minTexelOffset), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTexelOffset), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.minTexelGatherOffset), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxTexelGatherOffset), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.minInterpolationOffset), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxInterpolationOffset), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.subPixelInterpolationOffsetBits), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxFramebufferWidth), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxFramebufferHeight), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxFramebufferLayers), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.framebufferColorSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.framebufferDepthSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.framebufferStencilSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.framebufferNoAttachmentsSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxColorAttachments), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.sampledImageColorSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.sampledImageIntegerSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.sampledImageDepthSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.sampledImageStencilSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.storageImageSampleCounts), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxSampleMaskWords), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.timestampComputeAndGraphics), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.timestampPeriod), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxClipDistances), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxCullDistances), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.maxCombinedClipAndCullDistances), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.discreteQueuePriorities), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, pointfToStr(limits.pointSizeRange, 2),                     descs[descIndex].second);
        addLimitRow(descs[descIndex].first, pointfToStr(limits.lineWidthRange, 2),                     descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.pointSizeGranularity),               descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.lineWidthGranularity),               descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.strictLines),                        descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.standardSampleLocations),            descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.optimalBufferCopyOffsetAlignment),   descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.optimalBufferCopyRowPitchAlignment), descs[descIndex].second);
        addLimitRow(descs[descIndex].first, std::to_string(limits.nonCoherentAtomSize),                descs[descIndex].second);

        d.limits.resize(1);
        d.limits[0].valueRows = limitRows;
        d.limits[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",  "", -1  } );
        d.limits[0].header.cells.push_back( { Data::Cell::Style::Header, "Value", "", -1  } );

        auto addQueueRow = [&](
                std::vector<Data::Row>& rows,
                const std::string& familyIndex,
                const std::string& queueCount,
                const std::string& timestampValidBits,
                const std::string& flags,
                const std::string& minImageTransferGranularity)
        {
            rows.push_back(
            {{
                { Data::Cell::Style::NameLabel,  familyIndex,                 "", -1 },
                { Data::Cell::Style::ValueLabel, queueCount,                  "", -1 },
                { Data::Cell::Style::ValueLabel, timestampValidBits,          "", -1 },
                { Data::Cell::Style::ValueLabel, flags,                       "", -1 },
                { Data::Cell::Style::ValueLabel, minImageTransferGranularity, "", -1 },
            }});
        };

        std::vector<Data::Row> queueRows;
        for (const VulkanObjects::PhysicalDevice::Queue& q : device.queues)
            addQueueRow(queueRows,
                        std::to_string(q.queueFamilyIndex),
                        std::to_string(q.properties.queueCount),
                        std::to_string(q.properties.timestampValidBits),
                        toString(q.properties.queueFlags),
                        toString(q.properties.minImageTransferGranularity));

        d.queues.resize(1);
        d.queues[0].valueRows = queueRows;
        d.queues[0].header.cells.push_back( { Data::Cell::Style::Header, "Family Index",                    "", -1  } );
        d.queues[0].header.cells.push_back( { Data::Cell::Style::Header, "Queue Count",                     "", -1  } );
        d.queues[0].header.cells.push_back( { Data::Cell::Style::Header, "Capabilities",                    "", -1  } );
        d.queues[0].header.cells.push_back( { Data::Cell::Style::Header, "Min Image Transfer\nGranularity", "", -1  } );
        d.queues[0].header.cells.push_back( { Data::Cell::Style::Header, "Timestamp Valid\nBits",           "", -1  } );


        auto addMemoryRow = [&](
                std::vector<Data::Row>& rows,
                const std::string& heapIndex,
                const std::string& size,
                const std::string& properties,
                const std::string& flags)
        {
            rows.push_back(
            {{
                { Data::Cell::Style::NameLabel,  heapIndex,  "", -1 },
                { Data::Cell::Style::ValueLabel, size,       "", -1 },
                { Data::Cell::Style::ValueLabel, properties, "", -1 },
                { Data::Cell::Style::ValueLabel, flags,      "", -1 },
            }});
        };

        std::vector<Data::Row> memoryRows;
        for (int i = 0; i < int(device.memoryProperties.memoryHeapCount); ++i)
        {
            VkMemoryHeap heap = device.memoryProperties.memoryHeaps[i];
            std::string size = std::to_string(float(heap.size) / float(1024*1024*1024)) + " GB";
            std::string heapIndex = std::to_string(i);
            std::string  properties;
            if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                properties += "Device Local";
            if (heap.flags & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHX)
            {
                if (properties.size())
                    properties += " | ";
                properties += "Multi Instance";
            }
            std::string flags;

            for (int k = 0; k < int(device.memoryProperties.memoryTypeCount); ++k)
            {
                VkMemoryType type = device.memoryProperties.memoryTypes[k];
                if (type.heapIndex != i)
                    continue;


                std::string  typeFlags;
                if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
                    typeFlags += "Device Local";
                if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
                {
                    if (typeFlags.size())
                        typeFlags += " | ";
                    typeFlags += "Host Visible";
                }
                if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
                {
                    if (typeFlags.size())
                        typeFlags += " | ";
                    typeFlags += "Host Coherent";
                }
                if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
                {
                    if (typeFlags.size())
                        typeFlags += " | ";
                    typeFlags += "Host Cached";
                }
                if (type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
                {
                    if (typeFlags.size())
                        typeFlags += " | ";
                    typeFlags += "Lazily Allocated";
                }

                if (flags.size())
                    flags += "\n";
                flags += typeFlags;
            }

            addMemoryRow(memoryRows, heapIndex, size, properties, flags);
        }


        d.memories.resize(1);
        d.memories[0].valueRows = memoryRows;
        d.memories[0].header.cells.push_back( { Data::Cell::Style::Header, "Heap Index",  "", -1  } );
        d.memories[0].header.cells.push_back( { Data::Cell::Style::Header, "Size",        "", -1  } );
        d.memories[0].header.cells.push_back( { Data::Cell::Style::Header, "Properties",  "", -1  } );
        d.memories[0].header.cells.push_back( { Data::Cell::Style::Header, "Flags",       "", -1  } );

        const std::vector<std::pair<std::string, std::string>> formatStrings =
            readFormat("://descriptions/formats.txt");

        auto addFormatRow = [&](
                std::vector<Data::Row>& rows,
                const std::string& name,
                const std::string& desc,
                const std::string& linearTilingFeatures,
                const std::string& optimalTilingFeatures,
                const std::string& bufferFeatures)
        {
            rows.push_back(
            {{
                { Data::Cell::Style::NameLabel,  name,                  desc, -1 },
                { Data::Cell::Style::ValueLabel, linearTilingFeatures,  "",   -1 },
                { Data::Cell::Style::ValueLabel, optimalTilingFeatures, "",   -1 },
                { Data::Cell::Style::ValueLabel, bufferFeatures,        "",   -1 },
            }});
        };

        std::vector<Data::Row> formatRows;
        for (int i = 0; i < device.formats.size(); ++i)
        {
            const VulkanObjects::Format& f = device.formats[i];
            addFormatRow(formatRows,
                formatStrings[i].first,
                formatStrings[i].second,
                formatFeature(f.properties.linearTilingFeatures),
                formatFeature(f.properties.optimalTilingFeatures),
                formatFeature(f.properties.bufferFeatures));
        }

        d.formats.resize(1);
        d.formats[0].valueRows = formatRows;
        d.formats[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",            "", -1  } );
        d.formats[0].header.cells.push_back( { Data::Cell::Style::Header, "Linear Tiling",   "", -1  } );
        d.formats[0].header.cells.push_back( { Data::Cell::Style::Header, "Optimal Tiling",  "", -1  } );
        d.formats[0].header.cells.push_back( { Data::Cell::Style::Header, "Buffer features", "", -1  } );

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

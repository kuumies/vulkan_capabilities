/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_capabilities_controller.h"

/* -------------------------------------------------------------------------- */

#include <functional>
#include <iostream>
#include <sstream>
#include <QtCore/QString>

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_data.h"
#include "vk_capabilities_main_window.h"
#include "vk_capabilities_variable_description.h"
#include "vk_helper.h"
#include "vk_stringify.h"
#include "vk_instance.h"

namespace kuu
{
namespace vk_capabilities
{
namespace
{

/* -------------------------------------------------------------------------- *
   Creates the capabilities data from the Vulkan objects.
 * -------------------------------------------------------------------------- */
std::shared_ptr<Data> createCapabilitiesData(
    std::shared_ptr<vk::Instance> instance)
{
    using namespace vk::stringify;

    std::shared_ptr<Data> out = std::make_shared<Data>();
    if (instance->instance == VK_NULL_HANDLE)
        return out; // No Vulkan implementation
    out->hasVulkan = true;

    for (const vk::PhysicalDevice& device : instance->physicalDevices)
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
        for (const VkExtensionProperties& ex : instance->availableExtensions)
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
        for (const VkLayerProperties& l : instance->availableLayers)
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

        // Transforms a variable name from 'myVeryOwnVariable' to
        // 'My Very Own Variable'.
        // BUG: issues with array indices
        auto transformLimitVariable = [](const std::string& inStd)
        {
            QString in = QString::fromStdString(inStd);
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
            return out.toStdString();
        };

        VariableDescriptions limitVariableDesc("://descriptions/limits.txt", transformLimitVariable);
        std::vector<VariableDescriptions::VariableDescription> descs =
            limitVariableDesc.variableDescriptions();

        const VkPhysicalDeviceLimits& limits = device.properties.limits;
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

        addLimitRow(descs[descIndex].name, std::to_string(limits.maxImageDimension1D), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxImageDimension2D), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxImageDimension3D), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxImageDimensionCube), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxImageArrayLayers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTexelBufferElements), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxUniformBufferRange), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxStorageBufferRange), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPushConstantsSize), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxMemoryAllocationCount), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxSamplerAllocationCount), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.bufferImageGranularity), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.sparseAddressSpaceSize), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxBoundDescriptorSets), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorSamplers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorUniformBuffers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorStorageBuffers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorSampledImages), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorStorageImages), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorInputAttachments), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxPerStageResources), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetSamplers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetUniformBuffers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetUniformBuffersDynamic), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetStorageBuffers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetStorageBuffersDynamic), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetSampledImages), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetStorageImages), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetInputAttachments), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxVertexInputAttributes), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxVertexInputBindings), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxVertexInputAttributeOffset), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxVertexInputBindingStride), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxVertexOutputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationGenerationLevel), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationPatchSize), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlPerVertexInputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlPerVertexOutputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlPerPatchOutputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlTotalOutputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationEvaluationInputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTessellationEvaluationOutputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxGeometryShaderInvocations), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxGeometryInputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxGeometryOutputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxGeometryOutputVertices), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxGeometryTotalOutputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxFragmentInputComponents), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxFragmentOutputAttachments), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxFragmentDualSrcAttachments), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxFragmentCombinedOutputResources), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxComputeSharedMemorySize), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, pointToStr(limits.maxComputeWorkGroupCount, 3), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxComputeWorkGroupInvocations), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, pointToStr(limits.maxComputeWorkGroupSize, 3), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.subPixelPrecisionBits), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.subTexelPrecisionBits), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.mipmapPrecisionBits), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDrawIndexedIndexValue), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxDrawIndirectCount), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxSamplerLodBias), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxSamplerAnisotropy), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxViewports), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, pointToStr(limits.maxViewportDimensions, 2), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, rangeToStr(limits.viewportBoundsRange, 2), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.viewportSubPixelBits), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.minMemoryMapAlignment), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.minTexelBufferOffsetAlignment), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.minUniformBufferOffsetAlignment), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.minStorageBufferOffsetAlignment), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.minTexelOffset), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTexelOffset), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.minTexelGatherOffset), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxTexelGatherOffset), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.minInterpolationOffset), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxInterpolationOffset), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.subPixelInterpolationOffsetBits), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxFramebufferWidth), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxFramebufferHeight), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxFramebufferLayers), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.framebufferColorSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.framebufferDepthSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.framebufferStencilSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.framebufferNoAttachmentsSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxColorAttachments), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.sampledImageColorSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.sampledImageIntegerSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.sampledImageDepthSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.sampledImageStencilSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.storageImageSampleCounts), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxSampleMaskWords), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.timestampComputeAndGraphics), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.timestampPeriod), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxClipDistances), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxCullDistances), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.maxCombinedClipAndCullDistances), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.discreteQueuePriorities), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, pointfToStr(limits.pointSizeRange, 2),                     descs[descIndex].description);
        addLimitRow(descs[descIndex].name, pointfToStr(limits.lineWidthRange, 2),                     descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.pointSizeGranularity),               descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.lineWidthGranularity),               descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.strictLines),                        descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.standardSampleLocations),            descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.optimalBufferCopyOffsetAlignment),   descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.optimalBufferCopyRowPitchAlignment), descs[descIndex].description);
        addLimitRow(descs[descIndex].name, std::to_string(limits.nonCoherentAtomSize),                descs[descIndex].description);

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
        for (int familyIndex = 0; familyIndex < device.queueFamilies.size(); ++familyIndex)
        {
            const VkQueueFamilyProperties& q = device.queueFamilies[familyIndex];
            addQueueRow(queueRows,
                        std::to_string(familyIndex),
                        std::to_string(q.queueCount),
                        std::to_string(q.timestampValidBits),
                        toString(q.queueFlags),
                        toString(q.minImageTransferGranularity));
        }

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

        VariableDescriptions formatVariableDesc("://descriptions/formats.txt");
        std::vector<VariableDescriptions::VariableDescription> formatStrings =
            formatVariableDesc.variableDescriptions();

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
            const std::pair<VkFormat, VkFormatProperties>& f = device.formats[i];
            addFormatRow(formatRows,
                formatStrings[i].name,
                formatStrings[i].description,
                formatFeature(f.second.linearTilingFeatures),
                formatFeature(f.second.optimalTilingFeatures),
                formatFeature(f.second.bufferFeatures));
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
    std::shared_ptr<vk::Instance> instance;
    // Capabilities data created from vulkan objects.
    std::shared_ptr<Data> capabilitiesData;
};

/* -------------------------------------------------------------------------- */

Controller::Controller()
    : impl(std::make_shared<Impl>())
{
    std::vector<std::string> extensions;
    if (vk::helper::isInstanceExtensionSupported(
            "VK_KHR_get_physical_device_properties2"))
    {
        extensions.push_back("VK_KHR_get_physical_device_properties2");
    }

    impl->instance = std::make_shared<vk::Instance>(extensions);
    impl->capabilitiesData = createCapabilitiesData(impl->instance);
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

/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::DataCreator class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_data_creator.h"
#include "vk_stringify.h"
#include "vk_capabilities_variable_description.h"
#include <sstream>
#include <QtCore/QString>

namespace kuu
{
namespace vk_capabilities
{
namespace
{

/* -------------------------------------------------------------------------- */

void addRow(std::vector<Data::Row>& rows,
            const std::string& name,
            const std::string& value)
{
    rows.push_back(
    {{
        { Data::Cell::Style::NameLabel,  name,  "", -1 },
        { Data::Cell::Style::ValueLabel, value, "", -1 },
    }});
}

/* -------------------------------------------------------------------------- */

void addRow(std::vector<Data::Row>& rows,
            const std::string& name,
            const std::string& value,
            const std::string& desc)
{
    rows.push_back(
    {{
        { Data::Cell::Style::NameLabel,  name,  desc, -1 },
        { Data::Cell::Style::ValueLabel, value, desc, -1 },
    }});
}

/* -------------------------------------------------------------------------- */

// Transforms a variable name from 'myVeryOwnVariable' to
// 'My Very Own Variable'.
// BUG: issues with array indices
std::string transformVariable(const std::string& inStd)
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

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getDeviceProperties(
    const vk::PhysicalDevice& device)
{
    using namespace vk::stringify;

    VariableDescriptions limitVariableDesc("://descriptions/properties.txt", transformVariable);
    std::vector<VariableDescriptions::VariableDescription> descs =
        limitVariableDesc.variableDescriptions();

    std::vector<Data::Row> rows;
    addRow(rows, "Name",                device.properties.deviceName,                   descs[0].description);
    addRow(rows, "Type",                toString(device.properties.deviceType),         descs[1].description);
    addRow(rows, "API Version",         versionNumber(device.properties.apiVersion),    descs[2].description);
    addRow(rows, "Driver Version",      versionNumber(device.properties.driverVersion), descs[3].description);
    addRow(rows, "Vendor ID",           hexValueToString(device.properties.vendorID),   descs[4].description);
    addRow(rows, "Device ID",           hexValueToString(device.properties.deviceID),   descs[5].description);
    addRow(rows, "Pipeline Cache UUID", toString(device.properties.pipelineCacheUUID),  descs[6].description);

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = rows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Property",  "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Value",     "", -1  } );

    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getDeviceExtentions(
    const vk::Instance& instance,
    const vk::PhysicalDevice& device)
{
    std::vector<Data::Row> instanceExtensionRows;
    for (const VkExtensionProperties& ex : instance.availableExtensions)
        addRow(instanceExtensionRows,
               ex.extensionName,
               std::to_string(ex.specVersion));

    std::vector<Data::Row> deviceExtensionsRows;
    for (const VkExtensionProperties& ex : device.extensions)
        addRow(deviceExtensionsRows, ex.extensionName, std::to_string(ex.specVersion));

    std::vector<Data::Entry> out;
    out.resize(2);
    out[0].valueRows = instanceExtensionRows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Supported Instance Extension", "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Version",                      "", -1  } );
    out[1].valueRows = deviceExtensionsRows;
    out[1].header.cells.push_back( { Data::Cell::Style::Header, "Supported Device Extension", "", -1  } );
    out[1].header.cells.push_back( { Data::Cell::Style::Header, "Version",                    "", -1  } );
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getDeviceLayers(
    const vk::Instance& instance)
{
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
    for (const VkLayerProperties& l : instance.availableLayers)
        addLayerRow(instanceLayerRows,
                    l.layerName,
                    l.description,
                    vk::stringify::versionNumber(l.specVersion),
                    std::to_string(l.implementationVersion));

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = instanceLayerRows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",          "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Spec. Version", "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Impl. Version", "", -1  } );
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getFeatures(const vk::PhysicalDevice& device)
{
    VariableDescriptions limitVariableDesc("://descriptions/features.txt", transformVariable);
    std::vector<VariableDescriptions::VariableDescription> descs =
        limitVariableDesc.variableDescriptions();

    auto vkboolToStr = [](const VkBool32& b)
    { return  b == VK_TRUE ? "Supported" : "Unsupported"; };
    auto vkboolToStyle = [](const VkBool32& b)
    { return  b == VK_TRUE ? Data::Cell::Style::ValueLabelValid
                           : Data::Cell::Style::ValueLabelInvalid; };

    std::vector<Data::Row> featureRows;
    auto addFeatureRow = [&](
            const std::string& name,
            const VkBool32& b,
            const std::string& desc)
    {
        featureRows.push_back(
        {{
            { Data::Cell::Style::NameLabel, name,           desc, -1 },
            { vkboolToStyle(b),             vkboolToStr(b), desc, -1 },
        }});
    };

    int descIndex = 0;

    const VkPhysicalDeviceFeatures f = device.features;
    addFeatureRow("Robust Buffer Access",                         f.robustBufferAccess, descs[descIndex++].description);
    addFeatureRow("Full Draw Index Uint32",                       f.fullDrawIndexUint32, descs[descIndex++].description);
    addFeatureRow("Image Cube Array",                             f.imageCubeArray, descs[descIndex++].description);
    addFeatureRow("Independent Blend",                            f.independentBlend, descs[descIndex++].description);
    addFeatureRow("Geometry Shader",                              f.geometryShader, descs[descIndex++].description);
    addFeatureRow("Tesselation Shader",                           f.tessellationShader, descs[descIndex++].description);
    addFeatureRow("Sample Rate Shading",                          f.sampleRateShading, descs[descIndex++].description);
    addFeatureRow("Dual SRC Blend",                               f.dualSrcBlend, descs[descIndex++].description);
    addFeatureRow("Logic OP",                                     f.logicOp, descs[descIndex++].description);
    addFeatureRow("Multi Draw Indirect",                          f.multiDrawIndirect, descs[descIndex++].description);
    addFeatureRow("Draw Inderect First Instance",                 f.drawIndirectFirstInstance, descs[descIndex++].description);
    addFeatureRow("Depth CLamp",                                  f.depthClamp, descs[descIndex++].description);
    addFeatureRow("Depth Bias Clamp",                             f.depthBiasClamp, descs[descIndex++].description);
    addFeatureRow("Fill Mode Non Solid",                          f.fillModeNonSolid, descs[descIndex++].description);
    addFeatureRow("Depth Bounds",                                 f.depthBounds, descs[descIndex++].description);
    addFeatureRow("Wide Lines",                                   f.wideLines, descs[descIndex++].description);
    addFeatureRow("Large Points",                                 f.largePoints, descs[descIndex++].description);
    addFeatureRow("Alpha To One",                                 f.alphaToOne, descs[descIndex++].description);
    addFeatureRow("Multi Viewport",                               f.multiViewport, descs[descIndex++].description);
    addFeatureRow("Sampler Anisotropy",                           f.samplerAnisotropy, descs[descIndex++].description);
    addFeatureRow("Texture Compression ETC2",                     f.textureCompressionETC2, descs[descIndex++].description);
    addFeatureRow("Texture Compression ASTC_LDR",                 f.textureCompressionASTC_LDR, descs[descIndex++].description);
    addFeatureRow("Texture Compression BC",                       f.textureCompressionBC, descs[descIndex++].description);
    addFeatureRow("Occlusion Query Precise",                      f.occlusionQueryPrecise, descs[descIndex++].description);
    addFeatureRow("Pipeline Staticstics Query",                   f.pipelineStatisticsQuery, descs[descIndex++].description);
    addFeatureRow("Vertex Pipeline Stores and Atomics",           f.vertexPipelineStoresAndAtomics, descs[descIndex++].description);
    addFeatureRow("Fragment Stores and Atomics",                  f.fragmentStoresAndAtomics, descs[descIndex++].description);
    addFeatureRow("Shader Tesselation And Geometry Point Size",   f.shaderTessellationAndGeometryPointSize, descs[descIndex++].description);
    addFeatureRow("Shader Image Gather Extended",                 f.shaderImageGatherExtended, descs[descIndex++].description);
    addFeatureRow("Shader Storage Image Extended Formats",        f.shaderStorageImageExtendedFormats, descs[descIndex++].description);
    addFeatureRow("Shader Storage Image Multisample",             f.shaderStorageImageMultisample, descs[descIndex++].description);
    addFeatureRow("Shader Storage Image Read Without Format",     f.shaderStorageImageReadWithoutFormat, descs[descIndex++].description);
    addFeatureRow("Shader Storage image Write Without Format",    f.shaderStorageImageWriteWithoutFormat, descs[descIndex++].description);
    addFeatureRow("Shader Uniform Buffer Array Dynamic Indexing", f.shaderUniformBufferArrayDynamicIndexing, descs[descIndex++].description);
    addFeatureRow("Shader Sampled Array Dynamic Indexing",        f.shaderSampledImageArrayDynamicIndexing, descs[descIndex++].description);
    addFeatureRow("Shader Storage Buffer Array Dynamic Indexing", f.shaderStorageBufferArrayDynamicIndexing, descs[descIndex++].description);
    addFeatureRow("Shader Storage Image Array Dynamic Indexing",  f.shaderStorageImageArrayDynamicIndexing, descs[descIndex++].description);
    addFeatureRow("Shader Clip Distance",                         f.shaderClipDistance, descs[descIndex++].description);
    addFeatureRow("Shader Cull Distance",                         f.shaderCullDistance, descs[descIndex++].description);
    addFeatureRow("Shader Float64",                               f.shaderFloat64, descs[descIndex++].description);
    addFeatureRow("Shader Int64",                                 f.shaderInt64, descs[descIndex++].description);
    addFeatureRow("Shader Int16",                                 f.shaderInt16, descs[descIndex++].description);
    addFeatureRow("Shader Resource Residency",                    f.shaderResourceResidency, descs[descIndex++].description);
    addFeatureRow("Shader Resource Min LOD",                      f.shaderResourceMinLod, descs[descIndex++].description);
    addFeatureRow("Sparse Binding",                               f.sparseBinding, descs[descIndex++].description);
    addFeatureRow("Sparse Residency Buffer",                      f.sparseResidencyBuffer, descs[descIndex++].description);
    addFeatureRow("Sparse Residency Image 2D",                    f.sparseResidencyImage2D, descs[descIndex++].description);
    addFeatureRow("Sparse Residency Image 3D",                    f.sparseResidencyImage3D, descs[descIndex++].description);
    addFeatureRow("Sparse Residency 2 Samples",                   f.sparseResidency2Samples, descs[descIndex++].description);
    addFeatureRow("Sparse Residency 4 Samples",                   f.sparseResidency4Samples, descs[descIndex++].description);
    addFeatureRow("Sparse Residency 8 Samples",                   f.sparseResidency8Samples, descs[descIndex++].description);
    addFeatureRow("Sparse Residency 16 Samples",                  f.sparseResidency16Samples, descs[descIndex++].description);
    addFeatureRow("Sparse Residency Aliased",                     f.sparseResidencyAliased, descs[descIndex++].description);
    addFeatureRow("Variable Multisample Rate",                    f.variableMultisampleRate, descs[descIndex++].description);
    addFeatureRow("Inherited Queries",                            f.inheritedQueries, descs[descIndex++].description);

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = featureRows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",           "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Support Status", "", -1  } );
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getLimits(const vk::PhysicalDevice& device)
{
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

    VariableDescriptions limitVariableDesc("://descriptions/limits.txt", transformVariable);
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

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = limitRows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",  "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Value", "", -1  } );
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getQueues(const vk::PhysicalDevice& device)
{
    auto addQueueRow = [&](
            std::vector<Data::Row>& rows,
            const std::string& familyIndex,
            const std::string& queueCount,
            const std::string& presentation,
            const std::string& timestampValidBits,
            const std::string& flags,
            const std::string& minImageTransferGranularity)
    {
        rows.push_back(
        {{
            { Data::Cell::Style::NameLabel,  familyIndex,                 "", -1 },
            { Data::Cell::Style::ValueLabel, queueCount,                  "", -1 },
            { Data::Cell::Style::ValueLabel, presentation,                "", -1 },
            { Data::Cell::Style::ValueLabel, timestampValidBits,          "", -1 },
            { Data::Cell::Style::ValueLabel, flags,                       "", -1 },
            { Data::Cell::Style::ValueLabel, minImageTransferGranularity, "", -1 },
        }});
    };

    std::vector<Data::Row> queueRows;
    for (int familyIndex = 0; familyIndex < device.queueFamilies.size(); ++familyIndex)
    {
        const bool presentation = device.queuePresentation[familyIndex];
        const VkQueueFamilyProperties& q = device.queueFamilies[familyIndex];
        addQueueRow(queueRows,
                    std::to_string(familyIndex),
                    std::to_string(q.queueCount),
                    presentation ? "Supported" : "Unsupported",
                    std::to_string(q.timestampValidBits),
                    vk::stringify::toString(q.queueFlags),
                    vk::stringify::toString(q.minImageTransferGranularity));
    }

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = queueRows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Family Index",                    "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Queue Count",                     "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Presentation",                    "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Capabilities",                    "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Min Image Transfer\nGranularity", "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Timestamp Valid\nBits",           "", -1  } );
    return out;

}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getMemory(const vk::PhysicalDevice& device)
{
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


    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = memoryRows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Heap Index",  "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Size",        "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Properties",  "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Flags",       "", -1  } );
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getFormats(const vk::PhysicalDevice& device)
{
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
            vk::stringify::formatFeature(f.second.linearTilingFeatures),
            vk::stringify::formatFeature(f.second.optimalTilingFeatures),
            vk::stringify::formatFeature(f.second.bufferFeatures));
    }

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = formatRows;
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Name",            "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Linear Tiling",   "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Optimal Tiling",  "", -1  } );
    out[0].header.cells.push_back( { Data::Cell::Style::Header, "Buffer features", "", -1  } );
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getSurface(
    const vk::PhysicalDevice& device,
    const vk::SurfaceWidget& surface)
{
    std::vector<Data::Entry> out;
    return out;
}

/* -------------------------------------------------------------------------- */

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct DataCreator::Impl
{
    // Result data.
    std::shared_ptr<Data> data;
};

/* -------------------------------------------------------------------------- */

DataCreator::DataCreator(
    const vk::Instance& instance,
    const vk::SurfaceWidget& surfaceWidget)
    : impl(std::make_shared<Impl>())
{
    impl->data = std::make_shared<Data>();

    if (instance.instance == VK_NULL_HANDLE)
        return; // No Vulkan implementation

    impl->data->hasVulkan = true;

    for (const vk::PhysicalDevice& device : instance.physicalDevices)
    {
        Data::PhysicalDeviceData d;
        d.name       = device.properties.deviceName;
        d.properties = getDeviceProperties(device);
        d.properties[0].valueRows[0].cells[1].value = d.name;
        d.extensions = getDeviceExtentions(instance, device);
        d.layers     = getDeviceLayers(instance);
        d.features   = getFeatures(device);
        d.limits     = getLimits(device);
        d.queues     = getQueues(device);
        d.memories   = getMemory(device);
        d.formats    = getFormats(device);
        d.surface    = getSurface(device, surfaceWidget);

        impl->data->physicalDeviceData.push_back(d);
    }
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<Data> DataCreator::data() const
{ return impl->data; }

} // namespace vk_capabilities
} // namespace kuu

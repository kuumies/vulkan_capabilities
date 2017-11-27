/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::DataCreator class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_data_creator.h"
#include "vk_stringify.h"
#include "vk_capabilities_variable_description.h"

/* -------------------------------------------------------------------------- */

#include <sstream>

namespace kuu
{
namespace vk_capabilities
{
namespace
{

/* -------------------------------------------------------------------------- */

std::string vkboolToStr(const VkBool32& b)
{ return  b == VK_TRUE ? "Supported" : "Unsupported"; }

/* -------------------------------------------------------------------------- */

void addRow(std::vector<Data::Row>& rows,
            const std::string& name,
            const std::string& value)
{
    rows.push_back(
    {{
        { Data::Cell::Style::NameLabel,  name,  "" },
        { Data::Cell::Style::ValueLabel, value, "" },
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
        { Data::Cell::Style::NameLabel,  name,  desc },
        { Data::Cell::Style::ValueLabel, value, desc },
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

    VariableDescriptions limitVariableDesc("://descriptions/properties.txt");
    const std::vector<VariableDescriptions::Variable>& variables =
        limitVariableDesc.variables();

    const vk::PhysicalDeviceInfo& info = device.info();
    const VkPhysicalDeviceIDPropertiesKHR& idProperties = info.idProperties;

    std::vector<std::string> values =
    {
        info.properties.deviceName,
        vk::stringify::physicalDeviceType(info.properties.deviceType),
        vk::stringify::versionNumber(info.properties.apiVersion),
        vk::stringify::versionNumber(info.properties.driverVersion),
        vk::stringify::hexValueToString(info.properties.vendorID),
        vk::stringify::hexValueToString(info.properties.deviceID),
        vk::stringify::uuid(info.properties.pipelineCacheUUID),
    };

    if (info.hasExtensionsProperties)
    {
        std::vector<std::string> valuesExt =
        {
            vk::stringify::uuid(idProperties.deviceUUID),
            vk::stringify::uuid(idProperties.driverUUID),
            vk::stringify::luid(idProperties.deviceLUID),
            std::to_string(idProperties.deviceNodeMask),
            vkboolToStr(idProperties.deviceLUIDValid)
        };

        values.insert(values.end(), valuesExt.begin(), valuesExt.end());
    }

    std::vector<Data::Row> rows;
    for (int i = 0; i < values.size(); ++i)
    {
        rows.push_back(
        {{
            { Data::Cell::Style::NameLabel,  variables[i].name,  variables[i].description},
            { Data::Cell::Style::ValueLabel, values[i],          variables[i].description },
        }});
    }

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = rows;
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getDeviceExtentions(
    const vk::Instance& instance,
    const vk::PhysicalDevice& device)
{
    vk::InstanceInfo info;
    std::vector<Data::Row> instanceExtensionRows;
    for (const VkExtensionProperties& ex : info.extensions)
        addRow(instanceExtensionRows,
               ex.extensionName,
               std::to_string(ex.specVersion));

    const vk::PhysicalDeviceInfo& devInfo = device.info();
    std::vector<Data::Row> deviceExtensionsRows;
    for (const VkExtensionProperties& ex : devInfo.extensions)
        addRow(deviceExtensionsRows, ex.extensionName, std::to_string(ex.specVersion));

    std::vector<Data::Entry> out;
    out.resize(2);
    out[0].valueRows = instanceExtensionRows;
    out[1].valueRows = deviceExtensionsRows;
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
            { Data::Cell::Style::NameLabel,  name,        desc },
            { Data::Cell::Style::ValueLabel, specVersion, "" },
            { Data::Cell::Style::ValueLabel, implVersion, "" },
        }});
    };

    vk::InstanceInfo info;
    std::vector<Data::Row> instanceLayerRows;
    for (const VkLayerProperties& l : info.layers)
        addLayerRow(instanceLayerRows,
                    l.layerName,
                    l.description,
                    vk::stringify::versionNumber(l.specVersion),
                    std::to_string(l.implementationVersion));

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = instanceLayerRows;
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getFeatures(const vk::PhysicalDevice& device)
{
    VariableDescriptions limitVariableDesc("://descriptions/features.txt", transformVariable);
    std::vector<VariableDescriptions::Variable> descs =
        limitVariableDesc.variables();

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
            { Data::Cell::Style::NameLabel, name,           desc },
            { vkboolToStyle(b),             vkboolToStr(b), desc },
        }});
    };

    int descIndex = 0;

    const vk::PhysicalDeviceInfo& info = device.info();
    const VkPhysicalDeviceFeatures f = info.features;
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

    if (info.hasExtensionsFeatures)
    {
        VkPhysicalDeviceVariablePointerFeaturesKHR vp = info.featuresVariablePointer;
        addFeatureRow("Variable Pointers Storage Buffer", vp.variablePointersStorageBuffer, descs[descIndex++].description);
        addFeatureRow("Variable Pointers",                vp.variablePointers,              descs[descIndex++].description);

        VkPhysicalDeviceMultiviewFeaturesKHX mv = info.multiviewFeatures;
        addFeatureRow("Multiview",                    mv.multiview,                   descs[descIndex++].description);
        addFeatureRow("Multiview Geometry Shader",    mv.multiviewGeometryShader,     descs[descIndex++].description);
        addFeatureRow("Multiview Tesselation Shader", mv.multiviewTessellationShader, descs[descIndex++].description);

        VkPhysicalDevice16BitStorageFeaturesKHR s16 = info.features16BitStorage;
        addFeatureRow("Storage Buffer 16 bit Access",                s16.storageBuffer16BitAccess,           descs[descIndex++].description);
        addFeatureRow("Uniform and Storage Buffer 16 bit Access",    s16.uniformAndStorageBuffer16BitAccess, descs[descIndex++].description);
        addFeatureRow("Storage Push Constant 16",                    s16.storagePushConstant16,              descs[descIndex++].description);
        addFeatureRow("Storage Input/Output 16",                     s16.storageInputOutput16,               descs[descIndex++].description);

        VkPhysicalDeviceSamplerYcbcrConversionFeaturesKHR yuv = info.yuvSamplerFeatures;
        addFeatureRow("Sampler Y'CbCr Conversion", yuv.samplerYcbcrConversion,           descs[descIndex++].description);

        VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blend = info.blendFeatures;;
        addFeatureRow("Advanced Blend Coherent Operations", blend.advancedBlendCoherentOperations, descs[descIndex++].description);
    }

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = featureRows;
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
    std::vector<VariableDescriptions::Variable> descs =
        limitVariableDesc.variables();

    const vk::PhysicalDeviceInfo& info = device.info();
    const VkPhysicalDeviceLimits& limits = info.properties.limits;
    int descIndex = 0;

    std::vector<Data::Row> limitRows;
    auto addRow = [&](
            const std::string& name,
            const std::string& limit,
            const std::string& desc
            )
    {
        limitRows.push_back(
        {{
            { Data::Cell::Style::NameLabel, name,  desc },
            { Data::Cell::Style::ValueLabel,limit, ""   },
        }});
        descIndex++;
    };

    addRow(descs[descIndex].name, std::to_string(limits.maxImageDimension1D), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxImageDimension2D), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxImageDimension3D), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxImageDimensionCube), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxImageArrayLayers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTexelBufferElements), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxUniformBufferRange), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxStorageBufferRange), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPushConstantsSize), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxMemoryAllocationCount), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxSamplerAllocationCount), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.bufferImageGranularity), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.sparseAddressSpaceSize), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxBoundDescriptorSets), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorSamplers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorUniformBuffers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorStorageBuffers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorSampledImages), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorStorageImages), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPerStageDescriptorInputAttachments), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxPerStageResources), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetSamplers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetUniformBuffers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetUniformBuffersDynamic), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetStorageBuffers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetStorageBuffersDynamic), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetSampledImages), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetStorageImages), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDescriptorSetInputAttachments), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxVertexInputAttributes), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxVertexInputBindings), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxVertexInputAttributeOffset), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxVertexInputBindingStride), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxVertexOutputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationGenerationLevel), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationPatchSize), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlPerVertexInputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlPerVertexOutputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlPerPatchOutputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationControlTotalOutputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationEvaluationInputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTessellationEvaluationOutputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxGeometryShaderInvocations), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxGeometryInputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxGeometryOutputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxGeometryOutputVertices), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxGeometryTotalOutputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxFragmentInputComponents), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxFragmentOutputAttachments), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxFragmentDualSrcAttachments), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxFragmentCombinedOutputResources), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxComputeSharedMemorySize), descs[descIndex].description);
    addRow(descs[descIndex].name, pointToStr(limits.maxComputeWorkGroupCount, 3), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxComputeWorkGroupInvocations), descs[descIndex].description);
    addRow(descs[descIndex].name, pointToStr(limits.maxComputeWorkGroupSize, 3), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.subPixelPrecisionBits), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.subTexelPrecisionBits), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.mipmapPrecisionBits), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDrawIndexedIndexValue), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxDrawIndirectCount), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxSamplerLodBias), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxSamplerAnisotropy), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxViewports), descs[descIndex].description);
    addRow(descs[descIndex].name, pointToStr(limits.maxViewportDimensions, 2), descs[descIndex].description);
    addRow(descs[descIndex].name, rangeToStr(limits.viewportBoundsRange, 2), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.viewportSubPixelBits), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.minMemoryMapAlignment), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.minTexelBufferOffsetAlignment), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.minUniformBufferOffsetAlignment), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.minStorageBufferOffsetAlignment), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.minTexelOffset), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTexelOffset), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.minTexelGatherOffset), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxTexelGatherOffset), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.minInterpolationOffset), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxInterpolationOffset), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.subPixelInterpolationOffsetBits), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxFramebufferWidth), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxFramebufferHeight), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxFramebufferLayers), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.framebufferColorSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.framebufferDepthSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.framebufferStencilSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.framebufferNoAttachmentsSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxColorAttachments), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.sampledImageColorSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.sampledImageIntegerSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.sampledImageDepthSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.sampledImageStencilSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.storageImageSampleCounts), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxSampleMaskWords), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.timestampComputeAndGraphics), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.timestampPeriod), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxClipDistances), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxCullDistances), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.maxCombinedClipAndCullDistances), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.discreteQueuePriorities), descs[descIndex].description);
    addRow(descs[descIndex].name, pointfToStr(limits.pointSizeRange, 2),                     descs[descIndex].description);
    addRow(descs[descIndex].name, pointfToStr(limits.lineWidthRange, 2),                     descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.pointSizeGranularity),               descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.lineWidthGranularity),               descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.strictLines),                        descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.standardSampleLocations),            descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.optimalBufferCopyOffsetAlignment),   descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.optimalBufferCopyRowPitchAlignment), descs[descIndex].description);
    addRow(descs[descIndex].name, std::to_string(limits.nonCoherentAtomSize),                descs[descIndex].description);

    if (info.hasExtensionsProperties)
    {
        const VkPhysicalDeviceMultiviewPropertiesKHX&  multiview = info.multiviewProperties;
        addRow("Max Multiview View Count",     std::to_string(multiview.maxMultiviewViewCount),                descs[descIndex].description);
        addRow("Max Multiview Instance Index", std::to_string(multiview.maxMultiviewInstanceIndex),                descs[descIndex].description);

        VkPhysicalDeviceBlendOperationAdvancedPropertiesEXT blendProperties = info.blendProperties;
        addRow("Advanced Blend Max Color Attachments",       std::to_string(blendProperties.advancedBlendMaxColorAttachments),                descs[descIndex].description);
        addRow("Advanced Blend Independent Blend",           vkboolToStr(blendProperties.advancedBlendIndependentBlend),                descs[descIndex].description);
        addRow("Advanced Blend Non Premultiplied Src Color", vkboolToStr(blendProperties.advancedBlendNonPremultipliedSrcColor),                descs[descIndex].description);
        addRow("Advanced Blend Non Premultiplied Dst Color", vkboolToStr(blendProperties.advancedBlendNonPremultipliedDstColor),                descs[descIndex].description);
        addRow("Advanced Blend Correlated Overlap",          vkboolToStr(blendProperties.advancedBlendCorrelatedOverlap),                descs[descIndex].description);
        addRow("Advanced Blend All Operations",              vkboolToStr(blendProperties.advancedBlendAllOperations),                descs[descIndex].description);

        VkPhysicalDeviceDiscardRectanglePropertiesEXT  discardRectangleProperties  = info.discardRectangleProperties;
        addRow("Max Discard Rectangles",      std::to_string(discardRectangleProperties.maxDiscardRectangles),                descs[descIndex].description);

        VkPhysicalDevicePointClippingPropertiesKHR clippingProperties = info.clippingProperties;
        addRow("Point Clipping Behavior",   vk::stringify::pointClippingBehavior(clippingProperties.pointClippingBehavior),                descs[descIndex].description);

        VkPhysicalDevicePushDescriptorPropertiesKHR  pushDescriptorProperties = info.pushDescriptorProperties;
        addRow("Max Push Descriptors",      std::to_string(pushDescriptorProperties.maxPushDescriptors),                descs[descIndex].description);

        VkPhysicalDeviceSampleLocationsPropertiesEXT sampleLocationsProperties = info.sampleLocationsProperties;
        addRow("Sample Location Sample Counts",    vk::stringify::sampleCount(sampleLocationsProperties.sampleLocationSampleCounts),                descs[descIndex].description);
        addRow("Max Sample Location Grid Size",    vk::stringify::extent2D(sampleLocationsProperties.maxSampleLocationGridSize),                descs[descIndex].description);
        addRow("Sample Location Coordinate Range",
               "[" +
               std::to_string(sampleLocationsProperties.sampleLocationCoordinateRange[0]) + "," +
               std::to_string(sampleLocationsProperties.sampleLocationCoordinateRange[1]) + "]",                descs[descIndex].description);
        addRow("Sample Location Sub Pixel Bits", std::to_string(sampleLocationsProperties.sampleLocationSubPixelBits),                descs[descIndex].description);
        addRow("Variable Sample Locations",      vkboolToStr(sampleLocationsProperties.variableSampleLocations),                descs[descIndex].description);

        VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT samplerMinMaxProperties = info.samplerMinMaxProperties;
        addRow("Filter Minmax Single Component Formats", vkboolToStr(samplerMinMaxProperties.filterMinmaxSingleComponentFormats),                descs[descIndex].description);
        addRow("Filter Minmax Image Component Mapping",  vkboolToStr(samplerMinMaxProperties.filterMinmaxImageComponentMapping),                descs[descIndex].description);
    }

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = limitRows;
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
            { Data::Cell::Style::NameLabel,  familyIndex,                 "" },
            { Data::Cell::Style::ValueLabel, queueCount,                  "" },
            { Data::Cell::Style::ValueLabel, presentation,                "" },
            { Data::Cell::Style::ValueLabel, timestampValidBits,          "" },
            { Data::Cell::Style::ValueLabel, flags,                       "" },
            { Data::Cell::Style::ValueLabel, minImageTransferGranularity, "" },
        }});
    };

    const vk::PhysicalDeviceInfo& info = device.info();
    std::vector<Data::Row> queueRows;
    for (int familyIndex = 0; familyIndex < info.queueFamilies.size(); ++familyIndex)
    {
        const bool presentation = info.queuePresentation[familyIndex];
        const VkQueueFamilyProperties& q = info.queueFamilies[familyIndex];
        addQueueRow(queueRows,
                    std::to_string(familyIndex),
                    std::to_string(q.queueCount),
                    presentation ? "Supported" : "Unsupported",
                    std::to_string(q.timestampValidBits),
                    vk::stringify::queue(q.queueFlags),
                    vk::stringify::extent3D(q.minImageTransferGranularity));
    }

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = queueRows;
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
            { Data::Cell::Style::NameLabel,  heapIndex,  "" },
            { Data::Cell::Style::ValueLabel, size,       "" },
            { Data::Cell::Style::ValueLabel, properties, "" },
            { Data::Cell::Style::ValueLabel, flags,      "" },
        }});
    };

    const vk::PhysicalDeviceInfo& info = device.info();
    std::vector<Data::Row> memoryRows;
    for (int i = 0; i < int(info.memoryProperties.memoryHeapCount); ++i)
    {
        VkMemoryHeap heap = info.memoryProperties.memoryHeaps[i];
        std::string size = std::to_string(float(heap.size) / float(1024*1024*1024)) + " GB";
        std::string heapIndex = std::to_string(i);
        std::string  properties = vk::stringify::memoryHeap(heap.flags);

        std::string flags;

        for (int k = 0; k < int(info.memoryProperties.memoryTypeCount); ++k)
        {
            VkMemoryType type = info.memoryProperties.memoryTypes[k];
            if (type.heapIndex != i)
                continue;

            std::string typeFlags = vk::stringify::memoryProperty(type.propertyFlags);
            if (flags.size())
                flags += "\n";
            flags += typeFlags;
        }

        addMemoryRow(memoryRows, heapIndex, size, properties, flags);
    }

    std::vector<Data::Entry> out;
    out.resize(1);
    out[0].valueRows = memoryRows;
    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getFormats(const vk::PhysicalDevice& device)
{
    const vk::PhysicalDeviceInfo& info = device.info();

    VariableDescriptions formatVariableDesc("://descriptions/formats.txt");
    std::vector<VariableDescriptions::Variable> formatStrings =
        formatVariableDesc.variables();

    std::vector<Data::Row> formatLinearRows;
    for (int i = 0; i < info.formats.size(); ++i)
    {
        const std::pair<VkFormat, VkFormatProperties>& f = info.formats[i];
        formatLinearRows.push_back(
        {{
            { Data::Cell::Style::NameLabel,  formatStrings[i].name,                                        formatStrings[i].description},
            { Data::Cell::Style::ValueLabel, vk::stringify::formatFeature(f.second.linearTilingFeatures),  "" },
        }});
    }

    std::vector<Data::Row> formatOptimalRows;
    for (int i = 0; i < info.formats.size(); ++i)
    {
        const std::pair<VkFormat, VkFormatProperties>& f = info.formats[i];
        formatOptimalRows.push_back(
        {{
            { Data::Cell::Style::NameLabel,  formatStrings[i].name,                                        formatStrings[i].description},
            { Data::Cell::Style::ValueLabel, vk::stringify::formatFeature(f.second.optimalTilingFeatures),  "" },
        }});
    }

    std::vector<Data::Row> bufferRows;
    for (int i = 0; i < info.formats.size(); ++i)
    {
        const std::pair<VkFormat, VkFormatProperties>& f = info.formats[i];
        bufferRows.push_back(
        {{
            { Data::Cell::Style::NameLabel,  formatStrings[i].name,                                        formatStrings[i].description},
            { Data::Cell::Style::ValueLabel, vk::stringify::formatFeature(f.second.bufferFeatures),  "" },
        }});
    }

    std::vector<Data::Entry> out;
    out.resize(3);
    out[0].valueRows = formatLinearRows;
    out[1].valueRows = formatOptimalRows;
    out[2].valueRows = bufferRows;

    return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Data::Entry> getSurface(
    const vk::SurfaceProperties& surfaceProperties)
{   
    VariableDescriptions propertiesVariableDesc("://descriptions/surface_properties.txt");
    std::vector<VariableDescriptions::Variable> descStrings =
        propertiesVariableDesc.variables();

    const VkSurfaceCapabilitiesKHR caps = surfaceProperties.surfaceCapabilities;
    std::vector<std::string> values =
    {
        std::to_string(caps.minImageCount),
        std::to_string(caps.maxImageCount),
        vk::stringify::extent2D(caps.currentExtent),
        vk::stringify::extent2D(caps.minImageExtent),
        vk::stringify::extent2D(caps.maxImageExtent),
        std::to_string(caps.maxImageArrayLayers),
        vk::stringify::surfaceTransformFlags(caps.supportedTransforms),
        vk::stringify::surfaceTransformFlags(caps.currentTransform),
        vk::stringify::compositeAlphaFlags(caps.supportedCompositeAlpha),
        vk::stringify::imageUsageFlags(caps.supportedUsageFlags)
    };

    std::string modeStr;
    auto modes = surfaceProperties.presentModes;
    for (const auto mode : modes)
    {
        if (modeStr.size())
            modeStr += "\n";
        modeStr += vk::stringify::presentMode(mode);
    }
    values.push_back(modeStr);

    auto addRow = [&](
            std::vector<Data::Row>& rows,
            const std::string& name,
            const std::string& desc,
            const std::string& value)
    {
        rows.push_back(
        {{
            { Data::Cell::Style::NameLabel,  name,   desc },
            { Data::Cell::Style::ValueLabel, value,  ""   },
        }});
    };

    std::vector<Data::Row> propertiesRows;
    for (int i = 0; i < values.size(); ++i)
    {
        addRow(propertiesRows,
               descStrings[i].name,
               descStrings[i].description,
               values[i]);
    }

    auto surfaceFormats = surfaceProperties.surfaceFormats;
    std::vector<Data::Row> formatsRows;
    for (int i = 0; i < surfaceFormats.size(); ++i)
    {
        auto surfaceFormat = surfaceFormats[i];
        addRow(formatsRows,
            vk::stringify::format(surfaceFormat.format),
            "",
            vk::stringify::colorSpace(surfaceFormat.colorSpace));
    }

    std::vector<Data::Entry> out;
    out.resize(2);
    out[0].valueRows = propertiesRows;
    out[1].valueRows = formatsRows;
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
    const vk::SurfaceWidget& surfaceWidget,
    const std::vector<vk::SurfaceProperties> surfaceProperties)
    : impl(std::make_shared<Impl>())
{
    impl->data = std::make_shared<Data>();

    if (instance.handle() == VK_NULL_HANDLE)
        return; // No Vulkan implementation

    impl->data->hasVulkan = true;

    auto devices = instance.physicalDevices();
    for (int deviceIndex = 0; deviceIndex < devices.size(); ++deviceIndex)
    {
        const vk::PhysicalDevice& device = devices[deviceIndex];
        Data::PhysicalDeviceData d;
        d.name       = device.info().properties.deviceName;
        d.properties = getDeviceProperties(device);
        d.extensions = getDeviceExtentions(instance, device);
        d.layers     = getDeviceLayers(instance);
        d.features   = getFeatures(device);
        d.limits     = getLimits(device);
        d.queues     = getQueues(device);
        d.memories   = getMemory(device);
        d.formats    = getFormats(device);
        d.surface    = getSurface(surfaceProperties[deviceIndex]);

        impl->data->physicalDeviceData.push_back(d);
    }
}

/* -------------------------------------------------------------------------- */

std::shared_ptr<Data> DataCreator::data() const
{ return impl->data; }

} // namespace vk_capabilities
} // namespace kuu

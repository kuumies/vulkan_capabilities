/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::stringify namespace.
 * -------------------------------------------------------------------------- */
 
#include "vk_stringify.h"
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
#include <sstream>

namespace kuu
{
namespace vk
{
namespace stringify
{
namespace
{

/* -------------------------------------------------------------------------- */

std::string set(std::string& out,
                const int flag,
                const int flags,
                const std::string& txt,
                const std::string& seperator = ", ")
{
    if (flags & flag)
    {
        if (out.size())
            out += seperator;
        out += txt;
    }
    return out;
}

} // Anonymous namespace

/* -------------------------------------------------------------------------- */

std::string result(const VkResult result)
{
   static const std::map<VkResult, std::string> strings =
   {
       { VK_SUCCESS,                     "VK_SUCCESS"                     },
       { VK_ERROR_OUT_OF_HOST_MEMORY,    "VK_ERROR_OUT_OF_HOST_MEMORY"    },
       { VK_ERROR_OUT_OF_DEVICE_MEMORY,  "VK_ERROR_OUT_OF_DEVICE_MEMORY"  },
       { VK_ERROR_INITIALIZATION_FAILED, "VK_ERROR_INITIALIZATION_FAILED" },
       { VK_ERROR_LAYER_NOT_PRESENT,     "VK_ERROR_LAYER_NOT_PRESENT"     },
       { VK_ERROR_EXTENSION_NOT_PRESENT, "VK_ERROR_EXTENSION_NOT_PRESENT" },
       { VK_ERROR_INCOMPATIBLE_DRIVER,   "VK_ERROR_INCOMPATIBLE_DRIVER"   }
   };

   return strings.at(result);
}

/* -------------------------------------------------------------------------- */

std::string resultDesc(const VkResult result)
{
    static const std::map<VkResult, std::string> descriptions =
    {
        { VK_SUCCESS,                     "success"                            },
        { VK_ERROR_OUT_OF_HOST_MEMORY,    "a host memory allocation failed"    },
        { VK_ERROR_OUT_OF_DEVICE_MEMORY,  "a device memory allocation failed"  },
        { VK_ERROR_INITIALIZATION_FAILED, "initialization of an object could "
                                          "not be completed for implementation"
                                          "-specific reasons"                  },
        { VK_ERROR_LAYER_NOT_PRESENT,     "a requested layer is not present "
                                          "or could not be loaded."            },
        { VK_ERROR_EXTENSION_NOT_PRESENT, "a requested extension is not "
                                          "supported."                         },
        { VK_ERROR_INCOMPATIBLE_DRIVER,   "the requested version of Vulkan is "
                                          "not supported by the driver or is "
                                          "otherwise incompatible for "
                                          "implementation-specific reasons."   }
    };

    return descriptions.at(result);
}

/* -------------------------------------------------------------------------- */

std::string versionNumber(const uint32_t version)
{
    uint32_t major = VK_VERSION_MAJOR(version);
    uint32_t minor = VK_VERSION_MINOR(version);
    uint32_t patch = VK_VERSION_PATCH(version);

    return std::to_string(major) + "." +
           std::to_string(minor) + "." +
            std::to_string(patch);
}

/* -------------------------------------------------------------------------- */

std::string physicalDeviceType(const VkPhysicalDeviceType type)
{
    static const std::map<VkPhysicalDeviceType, std::string> strings =
    {
        { VK_PHYSICAL_DEVICE_TYPE_OTHER,          "Other"          },
        { VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, "Integrated GPU" },
        { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,   "Discrete GPU"   },
        { VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,    "Virtual GPU"    },
        { VK_PHYSICAL_DEVICE_TYPE_CPU,            "CPU"            }
    };

    return strings.at(type);
}

/* -------------------------------------------------------------------------- */

std::string physicalDeviceTypeDesc(const VkPhysicalDeviceType type)
{
    static const std::map<VkPhysicalDeviceType, std::string> descriptions =
    {
        { VK_PHYSICAL_DEVICE_TYPE_OTHER,          "the device does not match any other available types."                                  },
        { VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, "the device is typically one embedded in or tightly coupled with the host."             },
        { VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU,   "the device is typically a separate processor connected to the host via an interlink."  },
        { VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU,    "the device is typically a virtual node in a virtualization environment."               },
        { VK_PHYSICAL_DEVICE_TYPE_CPU,            "the device is typically running on the same processors as the host."                   }
    };

    return descriptions.at(type);
}

/* -------------------------------------------------------------------------- */

std::string uiiid(const uint8_t* uuid, int size)
{
    std::stringstream ss;
    for (int i = 0; i < size; i++)
    {
        uint8_t u = uuid[i];
        ss << std::setfill ('0')
           << std::setw(sizeof(uint8_t) * 2)
           << std::hex
           << int(u);

        if (i == 3 || i == 5 || i == 7)
            ss << "-";
    }
    return ss.str();
}

/* -------------------------------------------------------------------------- */

std::string queue(VkQueueFlags flags)
{
    std::string capabilitiesStr;
    set(capabilitiesStr, VK_QUEUE_GRAPHICS_BIT,       flags, "VK_QUEUE_GRAPHICS_BIT", "\n");
    set(capabilitiesStr, VK_QUEUE_COMPUTE_BIT,        flags, "VK_QUEUE_COMPUTE_BIT", "\n");
    set(capabilitiesStr, VK_QUEUE_TRANSFER_BIT,       flags, "VK_QUEUE_TRANSFER_BIT", "\n");
    set(capabilitiesStr, VK_QUEUE_SPARSE_BINDING_BIT, flags, "VK_QUEUE_SPARSE_BINDING_BIT", "\n");
    return capabilitiesStr;
}

/* -------------------------------------------------------------------------- */

std::string formatFeature(VkFormatFeatureFlags flags)
{
    std::string formatsStr;
    set(formatsStr, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT,               flags, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT,               flags, "VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT,        flags, "VK_FORMAT_FEATURE_STORAGE_IMAGE_ATOMIC_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT,        flags, "VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT, flags, "VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_ATOMIC_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT,               flags, "VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT,            flags, "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT,      flags, "VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,    flags, "VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_BLIT_SRC_BIT,                    flags, "VK_FORMAT_FEATURE_BLIT_SRC_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_BLIT_DST_BIT,                    flags, "VK_FORMAT_FEATURE_BLIT_DST_BIT", "\n");
    set(formatsStr, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT, flags, "VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT", "\n");
    return formatsStr;
}

/* -------------------------------------------------------------------------- */

std::string surfaceTransformFlags(VkSurfaceTransformFlagsKHR flags)
{
    std::string formatsStr;
    set(formatsStr, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,                     flags, "VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR,                    flags, "VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR,                   flags, "VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR,                   flags, "VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR,            flags, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR,  flags, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR, flags, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR, flags, "VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR", "\n");
    set(formatsStr, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,                      flags, "VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR", "\n");
    return formatsStr;
}

/* -------------------------------------------------------------------------- */

std::string compositeAlphaFlags(VkCompositeAlphaFlagsKHR flags)
{
    std::string formatsStr;
    set(formatsStr, VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,          flags, "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR", "\n");
    set(formatsStr, VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,  flags, "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR", "\n");
    set(formatsStr, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR, flags, "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR", "\n");
    set(formatsStr, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,         flags, "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR", "\n");
    return formatsStr;
}

/* -------------------------------------------------------------------------- */

std::string imageUsageFlags(VkImageUsageFlags flags)
{
    std::string formatsStr;
    set(formatsStr, VK_IMAGE_USAGE_TRANSFER_SRC_BIT,             flags, "VK_IMAGE_USAGE_TRANSFER_SRC_BIT", "\n");
    set(formatsStr, VK_IMAGE_USAGE_TRANSFER_DST_BIT,             flags, "VK_IMAGE_USAGE_TRANSFER_DST_BIT", "\n");
    set(formatsStr, VK_IMAGE_USAGE_SAMPLED_BIT,                  flags, "VK_IMAGE_USAGE_SAMPLED_BIT", "\n");
    set(formatsStr, VK_IMAGE_USAGE_STORAGE_BIT,                  flags, "VK_IMAGE_USAGE_STORAGE_BIT", "\n");
    set(formatsStr, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,         flags, "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT", "\n");
    set(formatsStr, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, flags, "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT", "\n");
    set(formatsStr, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,     flags, "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT", "\n");
    set(formatsStr, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,         flags, "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT", "\n");
    return formatsStr;
}

/* -------------------------------------------------------------------------- */

std::string memoryProperty(VkMemoryPropertyFlags flags)
{
    std::string formatsStr;
    set(formatsStr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,     flags, "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT", "\n");
    set(formatsStr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,     flags, "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT", "\n");
    set(formatsStr, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,    flags, "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT", "\n");
    set(formatsStr, VK_MEMORY_PROPERTY_HOST_CACHED_BIT,      flags, "VK_MEMORY_PROPERTY_HOST_CACHED_BIT", "\n");
    set(formatsStr, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, flags, "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT", "\n");
    return formatsStr;
}

/* -------------------------------------------------------------------------- */

std::string memoryHeap(VkMemoryHeapFlags flags)
{
    std::string str;
    set(str, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT,       flags, "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT", "\n");
    set(str, VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHX, flags, "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT_KHX", "\n");
    return str;
}

/* -------------------------------------------------------------------------- */

std::string extent2D(const VkExtent2D& e)
{
    return "[" + std::to_string(e.width)  + ", "
               + std::to_string(e.height) + "]";
}

/* -------------------------------------------------------------------------- */

std::string extent3D(const VkExtent3D& e)
{
    return "[" + std::to_string(e.width) + ", "
               + std::to_string(e.height) + ", "
            + std::to_string(e.depth) + "]";
}

/* -------------------------------------------------------------------------- */

std::string hexValueToString(uint32_t v)
{
    std::stringstream ss;
    ss << "0x" << std::hex << v;
    return ss.str();
}

/* -------------------------------------------------------------------------- */

std::string presentMode(VkPresentModeKHR mode)
{
    std::string str;
    switch(mode)
    {
        case VK_PRESENT_MODE_IMMEDIATE_KHR:                  str = "VK_PRESENT_MODE_IMMEDIATE_KHR"; break;
        case VK_PRESENT_MODE_MAILBOX_KHR:                    str = "VK_PRESENT_MODE_MAILBOX_KHR"; break;
        case VK_PRESENT_MODE_FIFO_KHR:                       str = "VK_PRESENT_MODE_FIFO_KHR"; break;
        case VK_PRESENT_MODE_FIFO_RELAXED_KHR:               str = "VK_PRESENT_MODE_FIFO_RELAXED_KHR"; break;
        case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:      str = "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR"; break;
        case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:  str = "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR"; break;
    }
    return str;
}

/* -------------------------------------------------------------------------- */

std::string format( VkFormat format)
{
    static const std::map<VkFormat, std::string> formatStrings =
    {
        { VK_FORMAT_UNDEFINED, "VK_FORMAT_UNDEFINED" },
        { VK_FORMAT_R4G4_UNORM_PACK8, "VK_FORMAT_R4G4_UNORM_PACK8" },
        { VK_FORMAT_R4G4B4A4_UNORM_PACK16, "VK_FORMAT_R4G4B4A4_UNORM_PACK16" },
        { VK_FORMAT_B4G4R4A4_UNORM_PACK16, "VK_FORMAT_B4G4R4A4_UNORM_PACK16" },
        { VK_FORMAT_R5G6B5_UNORM_PACK16, "VK_FORMAT_R5G6B5_UNORM_PACK16" },
        { VK_FORMAT_B5G6R5_UNORM_PACK16, "VK_FORMAT_B5G6R5_UNORM_PACK16" },
        { VK_FORMAT_R5G5B5A1_UNORM_PACK16, "VK_FORMAT_R5G5B5A1_UNORM_PACK16" },
        { VK_FORMAT_B5G5R5A1_UNORM_PACK16, "VK_FORMAT_B5G5R5A1_UNORM_PACK16" },
        { VK_FORMAT_A1R5G5B5_UNORM_PACK16, "VK_FORMAT_A1R5G5B5_UNORM_PACK16" },
        { VK_FORMAT_R8_UNORM, "VK_FORMAT_R8_UNORM" },
        { VK_FORMAT_R8_SNORM, "VK_FORMAT_R8_SNORM" },
        { VK_FORMAT_R8_USCALED, "VK_FORMAT_R8_USCALED" },
        { VK_FORMAT_R8_SSCALED, "VK_FORMAT_R8_SSCALED" },
        { VK_FORMAT_R8_UINT, "VK_FORMAT_R8_UINT" },
        { VK_FORMAT_R8_SINT, "VK_FORMAT_R8_SINT" },
        { VK_FORMAT_R8_SRGB, "VK_FORMAT_R8_SRGB" },
        { VK_FORMAT_R8G8_UNORM, "VK_FORMAT_R8G8_UNORM" },
        { VK_FORMAT_R8G8_SNORM, "VK_FORMAT_R8G8_SNORM" },
        { VK_FORMAT_R8G8_USCALED, "VK_FORMAT_R8G8_USCALED" },
        { VK_FORMAT_R8G8_SSCALED, "VK_FORMAT_R8G8_SSCALED" },
        { VK_FORMAT_R8G8_UINT, "VK_FORMAT_R8G8_UINT" },
        { VK_FORMAT_R8G8_SINT, "VK_FORMAT_R8G8_SINT" },
        { VK_FORMAT_R8G8_SRGB, "VK_FORMAT_R8G8_SRGB" },
        { VK_FORMAT_R8G8B8_UNORM, "VK_FORMAT_R8G8B8_UNORM" },
        { VK_FORMAT_R8G8B8_SNORM, "VK_FORMAT_R8G8B8_SNORM" },
        { VK_FORMAT_R8G8B8_USCALED, "VK_FORMAT_R8G8B8_USCALED" },
        { VK_FORMAT_R8G8B8_SSCALED, "VK_FORMAT_R8G8B8_SSCALED" },
        { VK_FORMAT_R8G8B8_UINT, "VK_FORMAT_R8G8B8_UINT" },
        { VK_FORMAT_R8G8B8_SINT, "VK_FORMAT_R8G8B8_SINT" },
        { VK_FORMAT_R8G8B8_SRGB, "VK_FORMAT_R8G8B8_SRGB" },
        { VK_FORMAT_B8G8R8_UNORM, "VK_FORMAT_B8G8R8_UNORM" },
        { VK_FORMAT_B8G8R8_SNORM, "VK_FORMAT_B8G8R8_SNORM" },
        { VK_FORMAT_B8G8R8_USCALED, "VK_FORMAT_B8G8R8_USCALED" },
        { VK_FORMAT_B8G8R8_SSCALED, "VK_FORMAT_B8G8R8_SSCALED" },
        { VK_FORMAT_B8G8R8_UINT, "VK_FORMAT_B8G8R8_UINT" },
        { VK_FORMAT_B8G8R8_SINT, "VK_FORMAT_B8G8R8_SINT" },
        { VK_FORMAT_B8G8R8_SRGB, "VK_FORMAT_B8G8R8_SRGB" },
        { VK_FORMAT_R8G8B8A8_UNORM, "VK_FORMAT_R8G8B8A8_UNORM" },
        { VK_FORMAT_R8G8B8A8_SNORM, "VK_FORMAT_R8G8B8A8_SNORM" },
        { VK_FORMAT_R8G8B8A8_USCALED, "VK_FORMAT_R8G8B8A8_USCALED" },
        { VK_FORMAT_R8G8B8A8_SSCALED, "VK_FORMAT_R8G8B8A8_SSCALED" },
        { VK_FORMAT_R8G8B8A8_UINT, "VK_FORMAT_R8G8B8A8_UINT" },
        { VK_FORMAT_R8G8B8A8_SINT, "VK_FORMAT_R8G8B8A8_SINT" },
        { VK_FORMAT_R8G8B8A8_SRGB, "VK_FORMAT_R8G8B8A8_SRGB" },
        { VK_FORMAT_B8G8R8A8_UNORM, "VK_FORMAT_B8G8R8A8_UNORM" },
        { VK_FORMAT_B8G8R8A8_SNORM, "VK_FORMAT_B8G8R8A8_SNORM" },
        { VK_FORMAT_B8G8R8A8_USCALED, "VK_FORMAT_B8G8R8A8_USCALED" },
        { VK_FORMAT_B8G8R8A8_SSCALED, "VK_FORMAT_B8G8R8A8_SSCALED" },
        { VK_FORMAT_B8G8R8A8_UINT, "VK_FORMAT_B8G8R8A8_UINT" },
        { VK_FORMAT_B8G8R8A8_SINT, "VK_FORMAT_B8G8R8A8_SINT" },
        { VK_FORMAT_B8G8R8A8_SRGB, "VK_FORMAT_B8G8R8A8_SRGB" },
        { VK_FORMAT_A8B8G8R8_UNORM_PACK32, "VK_FORMAT_A8B8G8R8_UNORM_PACK32" },
        { VK_FORMAT_A8B8G8R8_SNORM_PACK32, "VK_FORMAT_A8B8G8R8_SNORM_PACK32" },
        { VK_FORMAT_A8B8G8R8_USCALED_PACK32, "VK_FORMAT_A8B8G8R8_USCALED_PACK32" },
        { VK_FORMAT_A8B8G8R8_SSCALED_PACK32, "VK_FORMAT_A8B8G8R8_SSCALED_PACK32" },
        { VK_FORMAT_A8B8G8R8_UINT_PACK32, "VK_FORMAT_A8B8G8R8_UINT_PACK32" },
        { VK_FORMAT_A8B8G8R8_SINT_PACK32, "VK_FORMAT_A8B8G8R8_SINT_PACK32" },
        { VK_FORMAT_A8B8G8R8_SRGB_PACK32, "VK_FORMAT_A8B8G8R8_SRGB_PACK32" },
        { VK_FORMAT_A2R10G10B10_UNORM_PACK32, "VK_FORMAT_A2R10G10B10_UNORM_PACK32" },
        { VK_FORMAT_A2R10G10B10_SNORM_PACK32, "VK_FORMAT_A2R10G10B10_SNORM_PACK32" },
        { VK_FORMAT_A2R10G10B10_USCALED_PACK32, "VK_FORMAT_A2R10G10B10_USCALED_PACK32" },
        { VK_FORMAT_A2R10G10B10_SSCALED_PACK32, "VK_FORMAT_A2R10G10B10_SSCALED_PACK32" },
        { VK_FORMAT_A2R10G10B10_UINT_PACK32, "VK_FORMAT_A2R10G10B10_UINT_PACK32" },
        { VK_FORMAT_A2R10G10B10_SINT_PACK32, "VK_FORMAT_A2R10G10B10_SINT_PACK32" },
        { VK_FORMAT_A2B10G10R10_UNORM_PACK32, "VK_FORMAT_A2B10G10R10_UNORM_PACK32" },
        { VK_FORMAT_A2B10G10R10_SNORM_PACK32, "VK_FORMAT_A2B10G10R10_SNORM_PACK32" },
        { VK_FORMAT_A2B10G10R10_USCALED_PACK32, "VK_FORMAT_A2B10G10R10_USCALED_PACK32" },
        { VK_FORMAT_A2B10G10R10_SSCALED_PACK32, "VK_FORMAT_A2B10G10R10_SSCALED_PACK32" },
        { VK_FORMAT_A2B10G10R10_UINT_PACK32, "VK_FORMAT_A2B10G10R10_UINT_PACK32" },
        { VK_FORMAT_A2B10G10R10_SINT_PACK32, "VK_FORMAT_A2B10G10R10_SINT_PACK32" },
        { VK_FORMAT_R16_UNORM, "VK_FORMAT_R16_UNORM" },
        { VK_FORMAT_R16_SNORM, "VK_FORMAT_R16_SNORM" },
        { VK_FORMAT_R16_USCALED, "VK_FORMAT_R16_USCALED" },
        { VK_FORMAT_R16_SSCALED, "VK_FORMAT_R16_SSCALED" },
        { VK_FORMAT_R16_UINT, "VK_FORMAT_R16_UINT" },
        { VK_FORMAT_R16_SINT, "VK_FORMAT_R16_SINT" },
        { VK_FORMAT_R16_SFLOAT, "VK_FORMAT_R16_SFLOAT" },
        { VK_FORMAT_R16G16_UNORM, "VK_FORMAT_R16G16_UNORM" },
        { VK_FORMAT_R16G16_SNORM, "VK_FORMAT_R16G16_SNORM" },
        { VK_FORMAT_R16G16_USCALED, "VK_FORMAT_R16G16_USCALED" },
        { VK_FORMAT_R16G16_SSCALED, "VK_FORMAT_R16G16_SSCALED" },
        { VK_FORMAT_R16G16_UINT, "VK_FORMAT_R16G16_UINT" },
        { VK_FORMAT_R16G16_SINT, "VK_FORMAT_R16G16_SINT" },
        { VK_FORMAT_R16G16_SFLOAT, "VK_FORMAT_R16G16_SFLOAT" },
        { VK_FORMAT_R16G16B16_UNORM, "VK_FORMAT_R16G16B16_UNORM" },
        { VK_FORMAT_R16G16B16_SNORM, "VK_FORMAT_R16G16B16_SNORM" },
        { VK_FORMAT_R16G16B16_USCALED, "VK_FORMAT_R16G16B16_USCALED" },
        { VK_FORMAT_R16G16B16_SSCALED, "VK_FORMAT_R16G16B16_SSCALED" },
        { VK_FORMAT_R16G16B16_UINT, "VK_FORMAT_R16G16B16_UINT" },
        { VK_FORMAT_R16G16B16_SINT, "VK_FORMAT_R16G16B16_SINT" },
        { VK_FORMAT_R16G16B16_SFLOAT, "VK_FORMAT_R16G16B16_SFLOAT" },
        { VK_FORMAT_R16G16B16A16_UNORM, "VK_FORMAT_R16G16B16A16_UNORM" },
        { VK_FORMAT_R16G16B16A16_SNORM, "VK_FORMAT_R16G16B16A16_SNORM" },
        { VK_FORMAT_R16G16B16A16_USCALED, "VK_FORMAT_R16G16B16A16_USCALED" },
        { VK_FORMAT_R16G16B16A16_SSCALED, "VK_FORMAT_R16G16B16A16_SSCALED" },
        { VK_FORMAT_R16G16B16A16_UINT, "VK_FORMAT_R16G16B16A16_UINT" },
        { VK_FORMAT_R16G16B16A16_SINT, "VK_FORMAT_R16G16B16A16_SINT" },
        { VK_FORMAT_R16G16B16A16_SFLOAT, "VK_FORMAT_R16G16B16A16_SFLOAT" },
        { VK_FORMAT_R32_UINT, "VK_FORMAT_R32_UINT" },
        { VK_FORMAT_R32_SINT, "VK_FORMAT_R32_SINT" },
        { VK_FORMAT_R32_SFLOAT, "VK_FORMAT_R32_SFLOAT" },
        { VK_FORMAT_R32G32_UINT, "VK_FORMAT_R32G32_UINT" },
        { VK_FORMAT_R32G32_SINT, "VK_FORMAT_R32G32_SINT" },
        { VK_FORMAT_R32G32_SFLOAT, "VK_FORMAT_R32G32_SFLOAT" },
        { VK_FORMAT_R32G32B32_UINT, "VK_FORMAT_R32G32B32_UINT" },
        { VK_FORMAT_R32G32B32_SINT, "VK_FORMAT_R32G32B32_SINT" },
        { VK_FORMAT_R32G32B32_SFLOAT, "VK_FORMAT_R32G32B32_SFLOAT" },
        { VK_FORMAT_R32G32B32A32_UINT, "VK_FORMAT_R32G32B32A32_UINT" },
        { VK_FORMAT_R32G32B32A32_SINT, "VK_FORMAT_R32G32B32A32_SINT" },
        { VK_FORMAT_R32G32B32A32_SFLOAT, "VK_FORMAT_R32G32B32A32_SFLOAT" },
        { VK_FORMAT_R64_UINT, "VK_FORMAT_R64_UINT" },
        { VK_FORMAT_R64_SINT, "VK_FORMAT_R64_SINT" },
        { VK_FORMAT_R64_SFLOAT, "VK_FORMAT_R64_SFLOAT" },
        { VK_FORMAT_R64G64_UINT, "VK_FORMAT_R64G64_UINT" },
        { VK_FORMAT_R64G64_SINT, "VK_FORMAT_R64G64_SINT" },
        { VK_FORMAT_R64G64_SFLOAT, "VK_FORMAT_R64G64_SFLOAT" },
        { VK_FORMAT_R64G64B64_UINT, "VK_FORMAT_R64G64B64_UINT" },
        { VK_FORMAT_R64G64B64_SINT, "VK_FORMAT_R64G64B64_SINT" },
        { VK_FORMAT_R64G64B64_SFLOAT, "VK_FORMAT_R64G64B64_SFLOAT" },
        { VK_FORMAT_R64G64B64A64_UINT, "VK_FORMAT_R64G64B64A64_UINT" },
        { VK_FORMAT_R64G64B64A64_SINT, "VK_FORMAT_R64G64B64A64_SINT" },
        { VK_FORMAT_R64G64B64A64_SFLOAT, "VK_FORMAT_R64G64B64A64_SFLOAT" },
        { VK_FORMAT_B10G11R11_UFLOAT_PACK32, "VK_FORMAT_B10G11R11_UFLOAT_PACK32" },
        { VK_FORMAT_E5B9G9R9_UFLOAT_PACK32, "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32" },
        { VK_FORMAT_D16_UNORM, "VK_FORMAT_D16_UNORM" },
        { VK_FORMAT_X8_D24_UNORM_PACK32, "VK_FORMAT_X8_D24_UNORM_PACK32" },
        { VK_FORMAT_D32_SFLOAT, "VK_FORMAT_D32_SFLOAT" },
        { VK_FORMAT_S8_UINT, "VK_FORMAT_S8_UINT" },
        { VK_FORMAT_D16_UNORM_S8_UINT, "VK_FORMAT_D16_UNORM_S8_UINT" },
        { VK_FORMAT_D24_UNORM_S8_UINT, "VK_FORMAT_D24_UNORM_S8_UINT" },
        { VK_FORMAT_D32_SFLOAT_S8_UINT, "VK_FORMAT_D32_SFLOAT_S8_UINT" },
        { VK_FORMAT_BC1_RGB_UNORM_BLOCK, "VK_FORMAT_BC1_RGB_UNORM_BLOCK" },
        { VK_FORMAT_BC1_RGB_SRGB_BLOCK, "VK_FORMAT_BC1_RGB_SRGB_BLOCK" },
        { VK_FORMAT_BC1_RGBA_UNORM_BLOCK, "VK_FORMAT_BC1_RGBA_UNORM_BLOCK" },
        { VK_FORMAT_BC1_RGBA_SRGB_BLOCK, "VK_FORMAT_BC1_RGBA_SRGB_BLOCK" },
        { VK_FORMAT_BC2_UNORM_BLOCK, "VK_FORMAT_BC2_UNORM_BLOCK" },
        { VK_FORMAT_BC2_SRGB_BLOCK, "VK_FORMAT_BC2_SRGB_BLOCK" },
        { VK_FORMAT_BC3_UNORM_BLOCK, "VK_FORMAT_BC3_UNORM_BLOCK" },
        { VK_FORMAT_BC3_SRGB_BLOCK, "VK_FORMAT_BC3_SRGB_BLOCK" },
        { VK_FORMAT_BC4_UNORM_BLOCK, "VK_FORMAT_BC4_UNORM_BLOCK" },
        { VK_FORMAT_BC4_SNORM_BLOCK, "VK_FORMAT_BC4_SNORM_BLOCK" },
        { VK_FORMAT_BC5_UNORM_BLOCK, "VK_FORMAT_BC5_UNORM_BLOCK" },
        { VK_FORMAT_BC5_SNORM_BLOCK, "VK_FORMAT_BC5_SNORM_BLOCK" },
        { VK_FORMAT_BC6H_UFLOAT_BLOCK, "VK_FORMAT_BC6H_UFLOAT_BLOCK" },
        { VK_FORMAT_BC6H_SFLOAT_BLOCK, "VK_FORMAT_BC6H_SFLOAT_BLOCK" },
        { VK_FORMAT_BC7_UNORM_BLOCK, "VK_FORMAT_BC7_UNORM_BLOCK" },
        { VK_FORMAT_BC7_SRGB_BLOCK, "VK_FORMAT_BC7_SRGB_BLOCK" },
        { VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK, "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK" },
        { VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK, "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK" },
        { VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK, "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK" },
        { VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK, "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK" },
        { VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK, "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK" },
        { VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK, "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK" },
        { VK_FORMAT_EAC_R11_UNORM_BLOCK, "VK_FORMAT_EAC_R11_UNORM_BLOCK" },
        { VK_FORMAT_EAC_R11_SNORM_BLOCK, "VK_FORMAT_EAC_R11_SNORM_BLOCK" },
        { VK_FORMAT_EAC_R11G11_UNORM_BLOCK, "VK_FORMAT_EAC_R11G11_UNORM_BLOCK" },
        { VK_FORMAT_EAC_R11G11_SNORM_BLOCK, "VK_FORMAT_EAC_R11G11_SNORM_BLOCK" },
        { VK_FORMAT_ASTC_4x4_UNORM_BLOCK, "VK_FORMAT_ASTC_4x4_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_4x4_SRGB_BLOCK, "VK_FORMAT_ASTC_4x4_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_5x4_UNORM_BLOCK, "VK_FORMAT_ASTC_5x4_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_5x4_SRGB_BLOCK, "VK_FORMAT_ASTC_5x4_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_5x5_UNORM_BLOCK, "VK_FORMAT_ASTC_5x5_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_5x5_SRGB_BLOCK, "VK_FORMAT_ASTC_5x5_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_6x5_UNORM_BLOCK, "VK_FORMAT_ASTC_6x5_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_6x5_SRGB_BLOCK, "VK_FORMAT_ASTC_6x5_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_6x6_UNORM_BLOCK, "VK_FORMAT_ASTC_6x6_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_6x6_SRGB_BLOCK, "VK_FORMAT_ASTC_6x6_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_8x5_UNORM_BLOCK, "VK_FORMAT_ASTC_8x5_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_8x5_SRGB_BLOCK, "VK_FORMAT_ASTC_8x5_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_8x6_UNORM_BLOCK, "VK_FORMAT_ASTC_8x6_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_8x6_SRGB_BLOCK, "VK_FORMAT_ASTC_8x6_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_8x8_UNORM_BLOCK, "VK_FORMAT_ASTC_8x8_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_8x8_SRGB_BLOCK, "VK_FORMAT_ASTC_8x8_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_10x5_UNORM_BLOCK, "VK_FORMAT_ASTC_10x5_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_10x5_SRGB_BLOCK, "VK_FORMAT_ASTC_10x5_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_10x6_UNORM_BLOCK, "VK_FORMAT_ASTC_10x6_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_10x6_SRGB_BLOCK, "VK_FORMAT_ASTC_10x6_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_10x8_UNORM_BLOCK, "VK_FORMAT_ASTC_10x8_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_10x8_SRGB_BLOCK, "VK_FORMAT_ASTC_10x8_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_10x10_UNORM_BLOCK, "VK_FORMAT_ASTC_10x10_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_10x10_SRGB_BLOCK, "VK_FORMAT_ASTC_10x10_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_12x10_UNORM_BLOCK, "VK_FORMAT_ASTC_12x10_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_12x10_SRGB_BLOCK, "VK_FORMAT_ASTC_12x10_SRGB_BLOCK" },
        { VK_FORMAT_ASTC_12x12_UNORM_BLOCK, "VK_FORMAT_ASTC_12x12_UNORM_BLOCK" },
        { VK_FORMAT_ASTC_12x12_SRGB_BLOCK, "VK_FORMAT_ASTC_12x12_SRGB_BLOCK" },
        { VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG" },
        { VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG" },
        { VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG" },
        { VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG, "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG" },
        { VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG" },
        { VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG" },
        { VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG" },
        { VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG, "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG" },
        { VK_FORMAT_G8B8G8R8_422_UNORM_KHR, "VK_FORMAT_G8B8G8R8_422_UNORM_KHR" },
        { VK_FORMAT_B8G8R8G8_422_UNORM_KHR, "VK_FORMAT_B8G8R8G8_422_UNORM_KHR" },
        { VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR, "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM_KHR" },
        { VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR, "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM_KHR" },
        { VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR, "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM_KHR" },
        { VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR, "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM_KHR" },
        { VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR, "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM_KHR" },
        { VK_FORMAT_R10X6_UNORM_PACK16_KHR, "VK_FORMAT_R10X6_UNORM_PACK16_KHR" },
        { VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR, "VK_FORMAT_R10X6G10X6_UNORM_2PACK16_KHR" },
        { VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR, "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR" },
        { VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR, "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR" },
        { VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR, "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR" },
        { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR" },
        { VK_FORMAT_R12X4_UNORM_PACK16_KHR, "VK_FORMAT_R12X4_UNORM_PACK16_KHR" },
        { VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR, "VK_FORMAT_R12X4G12X4_UNORM_2PACK16_KHR" },
        { VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR, "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR" },
        { VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR, "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR" },
        { VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR, "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR" },
        { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR" },
        { VK_FORMAT_G16B16G16R16_422_UNORM_KHR, "VK_FORMAT_G16B16G16R16_422_UNORM_KHR" },
        { VK_FORMAT_B16G16R16G16_422_UNORM_KHR, "VK_FORMAT_B16G16R16G16_422_UNORM_KHR" },
        { VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR, "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM_KHR" },
        { VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR, "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM_KHR" },
        { VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR, "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM_KHR" },
        { VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR, "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM_KHR" },
    };

    return formatStrings.at(format);
}

/* -------------------------------------------------------------------------- */

std::string colorSpace(VkColorSpaceKHR colorSpace)
{
    std::string out;
    switch(colorSpace)
    {
        case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:          out = "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR"; break;
        case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:    out = "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT"; break;
        case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:    out = "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT"; break;
        case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT:           out = "VK_COLOR_SPACE_DCI_P3_LINEAR_EXT"; break;
        case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT:        out = "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT"; break;
        case VK_COLOR_SPACE_BT709_LINEAR_EXT:            out = "VK_COLOR_SPACE_BT709_LINEAR_EXT"; break;
        case VK_COLOR_SPACE_BT709_NONLINEAR_EXT:         out = "VK_COLOR_SPACE_BT709_NONLINEAR_EXT"; break;
        case VK_COLOR_SPACE_BT2020_LINEAR_EXT:           out = "VK_COLOR_SPACE_BT2020_LINEAR_EXT"; break;
        case VK_COLOR_SPACE_HDR10_ST2084_EXT:            out = "VK_COLOR_SPACE_HDR10_ST2084_EXT"; break;
        case VK_COLOR_SPACE_DOLBYVISION_EXT:             out = "VK_COLOR_SPACE_DOLBYVISION_EXT"; break;
        case VK_COLOR_SPACE_HDR10_HLG_EXT:               out = "VK_COLOR_SPACE_HDR10_HLG_EXT"; break;
        case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT:         out = "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT"; break;
        case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT:      out = "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT"; break;
        case VK_COLOR_SPACE_PASS_THROUGH_EXT:            out = "VK_COLOR_SPACE_PASS_THROUGH_EXT"; break;
        case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT: out = "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT"; break;
    }
    return out;
}

} // namespace stringify
} // namespace vk
} // namespace kuu

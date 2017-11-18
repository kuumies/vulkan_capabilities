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

std::string set(std::string& out, int flag, int flags, std::string txt, std::string seperator = ", ")
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

std::string toString(const VkResult result)
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

std::string toDescription(const VkResult result)
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

std::string toString(const VkPhysicalDeviceType type)
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

std::string toDescription(const VkPhysicalDeviceType type)
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

std::string toString(const uint8_t* uuid, int size)
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

std::string toString(VkQueueFlags flags)
{
    auto set = [flags](std::string& out, int flag, std::string txt)
    {
        if (flags & flag)
        {
            if (out.size())
                out += ", ";
            out += txt;
        }
    };

    std::string capabilitiesStr;
    set(capabilitiesStr, VK_QUEUE_GRAPHICS_BIT,       "Graphics");
    set(capabilitiesStr, VK_QUEUE_COMPUTE_BIT,        "Compute");
    set(capabilitiesStr, VK_QUEUE_TRANSFER_BIT,       "Transfer");
    set(capabilitiesStr, VK_QUEUE_SPARSE_BINDING_BIT, "Sparse Binding");
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

std::string toString(const VkExtent3D& e)
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

} // namespace stringify
} // namespace vk
} // namespace kuu

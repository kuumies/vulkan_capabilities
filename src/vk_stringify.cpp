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

} // namespace stringify
} // namespace vk
} // namespace kuu

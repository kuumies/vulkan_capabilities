/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Instance class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "vk_physical_device.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan instance RAII wrapper struct. Instance is created on constructor
   and destoryed on destructor.
 * -------------------------------------------------------------------------- */
struct Instance
{
    // Constructs the instance.
    Instance(const std::vector<std::string>& extensionNames =
                std::vector<std::string>(),
             bool validate = true);
    // Destroys the instance.
    ~Instance();

    // Returns true if the instance handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Vulkan instance handle. This a null handle if the system does  not
    // contain a Vulkan  implementation.
    VkInstance instance;
    
    // Validation callback. This is created if validate argument was set to
    // true during instance construction.
    VkDebugReportCallbackEXT callback;
    
    // Extension namess that were given in by user during construction.
    std::vector<std::string> extensionNames;

    // Instance extensions the implementaion supports. These are the
    // AVAILABLE extensions, not the ones given in during construction.
    std::vector<VkExtensionProperties> availableExtensions;
    // Instance layers the implementaion supports.
    std::vector<VkLayerProperties> availableLayers;

    // Physical devices.
    std::vector<PhysicalDevice> physicalDevices;

    // Returns true if the Vulkan implementation supports the given in
    // instance extensions. This can be asked before creating an instance.
    static bool isExtensionSupported(const std::string& extension);

    // Returns true if the Vulkan implementation supports the given in
    // layer. This can be asked before creating an instance.
    static bool isLayerSupported(const std::string& extension);
};

} // namespace vk
} // namespace kuu

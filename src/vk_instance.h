/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Instance class
 * ---------------------------------------------------------------- */

#pragma once

/* ---------------------------------------------------------------- */

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_physical_device.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   A Vulkan instance.
 * ---------------------------------------------------------------- */
class Instance
{
public:
    // Constructs the instance. A runtime exception is thrown if the
    // system does not support Vulkan.
    Instance(const std::string& applicationName,
             const std::string& engineName,
             const std::vector<std::string>& extensions,
             const std::vector<std::string>& layers);

    // Returns handle to instance.
    VkInstance handle() const;
    // Returns the physical devices.
    std::vector<PhysicalDevice> physicalDevices(VkSurfaceKHR surface) const;

    // Sets the validation layer enabled.
    void setValidationLeyerEnabled();

    // Returns true if the instance supports the given in extension.
    static bool isExtensionSupported(const std::string& extension);
    // Returns true if the instance supports the given in layer..
    static bool isLayerSupported(const std::string& layer);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu

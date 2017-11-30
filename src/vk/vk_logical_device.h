/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::LogicalDevice class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A Vulkan logical device wrapper class.
 * -------------------------------------------------------------------------- */
class LogicalDevice
{
public:
    // Queue family parameters, used to add a new queue to be created along
    // with the logical device.
    struct QueueFamilyParams
    {
        uint32_t queueFamilyIndex;
        uint32_t queueCount;
        float priority;
    };

    // Constructs the physical device from physical device.
    LogicalDevice(const VkPhysicalDevice& physicalDevice);

    // Sets and gets the logical device extension names to use.
    LogicalDevice& setExtensions(const std::vector<std::string>& extensions);
    std::vector<std::string> extensions() const;

    // Sets and gets the logical device layer names to use.
    LogicalDevice& setLayers(const std::vector<std::string>& layers);
    std::vector<std::string> layers() const;

    // Sets and gets the physical device features that should be enabled for
    // the logical device. By default this is the features struct from info.
    LogicalDevice& setFeatures(const VkPhysicalDeviceFeatures& deviceFeatures);
    VkPhysicalDeviceFeatures features() const;

    // Adds a queue of certain queue family to be created.
    LogicalDevice& addQueueFamily(const QueueFamilyParams& queue);
    LogicalDevice& addQueueFamily(uint32_t queueFamilyIndex,
                                  uint32_t queueCount,
                                  float priority);
    // Returns the queue family params.
    std::vector<QueueFamilyParams> queueFamilyParams() const;
    
    // Creates the logical device with the give in queue families.
    bool create();
    // Destroys the logical device.
    void destroy();

    // Returns true if the device is valid. Device is valid if the
    // logical device handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the handle.
    VkDevice handle() const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu

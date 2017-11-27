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
   Contains information of the Vulkan instance implementation.

   Note that this can be constructed without creating an instance. Creating
   an instance requires user to give in the extensions and layers to be used
   along with the Vulkan and the availabity of particular extension or layer
   can be asked from this struct.

   Example:

    const vk::InstanceInfo instanceInfo;
    if (instanceInfo.isExtensionSupported("VK_KHR_surface"))
    {
       // use extensions with instance
    }


 * -------------------------------------------------------------------------- */
struct InstanceInfo
{
    InstanceInfo();

    bool isExtensionSupported(const std::string& extension) const;
    bool isLayerSupported(const std::string& layer) const;

    std::vector<VkExtensionProperties> extensions;
    std::vector<VkLayerProperties> layers;
};

/* -------------------------------------------------------------------------- *
   A Vulkan instance wrapper class.

   Main purpose of an instance is to verify that the system has a Vulkan
   implementation and that there is a physical device which has the Vulkan
   ability. Instance is the root object of a Vulkan application.

   Remember that the instance needs to be deleted as last of the Vulkan
   objects. As this class uses shared data model care should be taken to
   follow that rule.

   User needs to manually call create function after giving in the possible
   extra data (e.g. application name, etc.).

   Instance class adds the validation layer and extensions it needs if user
   calls setValidateEnabled. This will also include a debug message callback
   where validation errors are printed into standard error output. Some im-
   plementations might not support the valiation layer, in that case if the
   instance is created with validtion enable the instance creation will fail.

   All errors are printend into standard erro output.

   Example:

      Instance instance;
      instance.setApplicationName("my_app")
              .setValidateEnabled(true);
      instance.create();
      if (!instance.isValid())
        return;

      // ...

      // Manually destroy instance. Automatic destruction happens after
      // last copy of instance goes out-of-scope. 
      // instance.destroy(); 

 * -------------------------------------------------------------------------- */
class Instance
{
public:
    // Constructs the instance. The Vulkan instance is not yet created.
    Instance();

    // Sets and gets the application name.
    Instance& setApplicationName(const std::string& name);
    std::string applicationName() const;

    // Sets and gets the engine name.
    Instance& setEngineName(const std::string& name);
    std::string engineName() const;

    // Set and get the used extensions.
    Instance& setExtensionNames(const std::vector<std::string>& extensionNames);
    std::vector<std::string> extensionNames() const;

    // Set and get the used layers.
    Instance& setLayerNames(const std::vector<std::string>& layerNames);
    std::vector<std::string> layerNames() const;

    // Set and get the validation layer status.
    Instance& setValidateEnabled(bool validate);
    bool isValidateEnabled() const;

    // Creates and destroys the instance.
    void create();
    void destroy();

    // Returns true if the instance handle is not a VK_NULL_HANDLE.
    bool isValid() const;

    // Returns the instance handle.
    VkInstance handle() const;

    // Returns the available physical devices.
    std::vector<PhysicalDevice> physicalDevices() const;
    // Returns physical device.
    PhysicalDevice physicalDevice(int index) const;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu

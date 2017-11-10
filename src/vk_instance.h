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
#include "vk_surface.h"

/* ---------------------------------------------------------------- */

struct GLFWwindow;

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   A Vulkan instance.
   
   Instance allows user to find out the devices with Vulkan-
   cabability, selecting the device to be used with rendering and
   creating a logical device for accessing the physical device.
      
   To create the instance user needs to set some (optional) infor-
   mation about the application:
   
    * application name
    * engine name
    * vulkan extensions to use (e.g. surface extension)
    * vulkan layers to use (e.g. debug layer)
    
   User can use static isExtensionSupported and isLayerSupported 
   before creating the instance to find out if the required extension 
   or layer is supported.
    
   Instance is created by calling create() function. If the 
   application information above changes then instance needs to 
   be recreated. Also all the objects that use the instance needs 
   to be recreated.
   
 * ---------------------------------------------------------------- */
class Instance
{
public:
    // Defines instance creation paramaters.
    struct Parameters
    {
        // Application name
        std::string applicationName;

        // Rendering engine name
        std::string engineName;

        // Instance extensions to be used. Note that surface
        // extensions are automatically set if createSurface
        // param is true.
        std::vector<std::string> extensions;

        // Instance layers to be used. Note that validation layer
        // is automatically created if createValidation param is
        // set to true.
        std::vector<std::string> layers;

        // Set true to create the validation layer and a debug
        // callback. The callback will print issues reported by
        // the validation layer into standard error output. If the
        // system does not support validation then the instance set
        // to invalid state.
        bool createValidationLayer = false;

        // Sets the extensions that are needed to create the surface.
        // If the system does not support surface then the instance
        // set to invalid state.
        bool createSurfaceExtensesions = false;
    };

    // Constructs the instance.
    Instance(const Parameters& params);

    // Returns true if the instance valid.
    bool isValid() const;

    // Returns handle to Vulkan instance. Handle is not valid if
    // the instance is not valid.
    VkInstance handle() const;
    // Creates a surface for a widnow. Surface is not valid if
    // the instance is not valid.
    Surface createSurface(GLFWwindow* window) const;
    
    // Returns all physical devices.
    std::vector<PhysicalDevice> physicalDevices() const;

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

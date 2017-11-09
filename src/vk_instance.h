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
    // Constructs the instance.
    Instance();

    // Sets the application name.
    Instance& setApplicationName(const std::string& applicationName);
    // Sets the engine name.
    Instance& setEngineName(const std::string& engineName);
    // Sets the extensions.
    Instance& setExtensions(std::vector<std::string>& extensions);
    // Sets the layers. If the layers contains the validation layer
    // then a debug callback is created. The callback will print
    // issues reported by the validation layer into standard error
    // output.
    Instance& setLayers(std::vector<std::string>& layers);
    // Sets that the validation layer is created.
    Instance& setCreateValidationLayer();
    // Sets that the surface is created for the window object.
    Instance& setCreateSurface(GLFWwindow* window);
    
    // Creates the instance.
    bool create();
    // Destroys the instance.
    bool destroy();
    
    // Returns handle to Vulkan instance. If the instance has not been 
    // created or it has been destroyd then a std::runtime_exection is
    // thrown.
    VkInstance handle() const;
    // Returns the surface. Surface is created only if the
    // setCreateSurface function was called before creating the
    // instance.
//    Surface surface() const;
    
    // Returns all physical devices. If the instance is has not been 
    // created or it has been deleted then a std::runtime_exception is 
    // thrown.
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

/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk:_test::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_test_controller.h"
#include <iostream>
#include <vector>
#include <ios>
#include <sstream>
#include <iomanip>

#include "ui/vk_test_main_window.h"
#include "ui/vk_test_widget.h"
#include "ui/vk_test_physical_device_selection_dialog.h"
#include "vk/vk_stringify.h"
#include "vk/vk_windows.h"

namespace kuu
{
namespace vk_test
{

/* -------------------------------------------------------------------------- */

Controller::Controller()
{}

/* -------------------------------------------------------------------------- */

Controller::~Controller()
{
    destroyInstance();
    delete mainWindow;
}

/* -------------------------------------------------------------------------- */

void Controller::runVulkanTest()
{
    mainWindow = new MainWindow();
    mainWindow->show();

    if (!createInstance())
        return;

    PhysicalDeviceSelectionDialog selectDevice;
    selectDevice.setInstance(instance);
    selectDevice.exec();

}

/* -------------------------------------------------------------------------- *
   Creates a Vulkan instance.
    
   There is no global Vulkan state. All the per-application state is stored
   in a instance. When an instance is created then the Vulkan is initialized.
   User can give in information about the application to Vulkan implementation.
    
   Instance creation functions does:

       1. verifies that an requested layers exits.
           -> VK_ERROR_LAYER_NOT_PRESENT
       2. verifies that requested extensions exits
           -> in implementation
           -> in any requested and now enabled layer
           -> in extensions (extension's depedancy to other extensions)
           -> VK_ERROR_EXTENSION_NOT_PRESENT
       3. creates an instance object
    
   See more:
   https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#initialization-instances
   https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
 * -------------------------------------------------------------------------- */
bool Controller::createInstance()
{
    std::vector<const char*> extensionNames;
    extensionNames.push_back("VK_KHR_get_physical_device_properties2");
    extensionNames.push_back("VK_KHR_surface");       // Surface
    extensionNames.push_back("VK_KHR_win32_surface"); // Surface (Windows OS)
    const uint32_t extensionCount =
        static_cast<uint32_t>(extensionNames.size());

    VkStructureType appInfoType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    VkApplicationInfo appInfo;
    appInfo.sType              = appInfoType;              // Must be this definition
    appInfo.pNext              = NULL;                     // No extension specific structures
    appInfo.pApplicationName   = "Vulkan Test";            // Name of the application
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version of the application
    appInfo.pEngineName        = "Vulkan Test Engine";     // Name of the "engine"
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0); // Version of the "engine"
    appInfo.apiVersion         = VK_API_VERSION_1_0;       // Version of the Vulkan API

    VkStructureType infoType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    VkInstanceCreateInfo info;
    info.sType                   = infoType;              // Must be this definition
    info.pNext                   = NULL;                  // No extension specific structures
    info.flags                   = 0;                     // Must be 0, reserved for future use
    info.pApplicationInfo        = &appInfo;              // Application information
    info.enabledLayerCount       = 0;                     // Count of requested layers
    info.ppEnabledLayerNames     = NULL;                  // Requested layer names
    info.enabledExtensionCount   = extensionCount;        // Count of requested extensions
    info.ppEnabledExtensionNames = extensionNames.data(); // Requested extension names

    const VkResult result = vkCreateInstance(
        &info,      // [in]  instance create info
        NULL,       // [in]  no allocation callback
        &instance); // [out] instance handle
        
    if (result == VK_SUCCESS)
        return true;
        
    std::cerr << __FUNCTION__
              << ": instance creation failed as "
              << vk::stringify::toString(result)
              << std::endl;

    return false;
}

/* -------------------------------------------------------------------------- *
   Destroys the instance.
   
   All the child object created to instance needs to be destroyed before the
   instance destruction.
   
   Instance destruction must be externally synchronized. VK test does not use
   threads to create Vulkan objects so no mutexed needed.

   Look like that the vkDestroyInstance does not set the intance object to be
   VK_NULL_HANDLE or NULL after destruction. Destroying the instance twice
   will lead to application crash.
 * -------------------------------------------------------------------------- */
void Controller::destroyInstance()
{
    vkDestroyInstance(
        instance, // [in] handle to instance
        NULL);    // [in] no allocation callback
    instance = VK_NULL_HANDLE;
}

} // namespace vk_test
} // namespace kuu

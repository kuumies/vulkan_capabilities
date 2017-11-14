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

#include "vk_stringify.h"

namespace kuu
{
namespace vk_test
{

/* -------------------------------------------------------------------------- */

Controller::Controller()
{
    if (!createInstance())
        return;
    enumeratePhysicalDevices();
}

/* -------------------------------------------------------------------------- */

Controller::~Controller()
{
    destroyInstance();
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
        &info,      // instance create info
        NULL,       // no allocation callback
        &instance); // created instance
        
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
        instance, // handle to instance
        NULL);    // no allocation callback
    instance = VK_NULL_HANDLE;
}

/* -------------------------------------------------------------------------- *
   Enumerates the physical devices.
   
   A physical device is usually a single device in a system. As spec says it
   could be made up of several individual hardware devices working together.
   I guess like when using SLI mode?
   
   See: https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#devsandqueues-physical-device-enumeration
 * -------------------------------------------------------------------------- */
void Controller::enumeratePhysicalDevices()
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(
        instance,             // Instance handle [in]
        &physicalDeviceCount, // Physical device count [out]
        NULL);                // Pointer to vector of physical devices, NULL
                              // so the physical device count is returned.
        
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    const VkResult result = vkEnumeratePhysicalDevices(
        instance,                // Instance handle [in]
        &physicalDeviceCount,    // Physical device count [in, out]
        physicalDevices.data()); // Pointer to vector of physical devices [out]
        
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": physical device enumeration failed as "
                  << vk::stringify::toDescription(result)
                  << std::endl;

        if (result != VK_INCOMPLETE)
            return;
    }
    
    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(
            physicalDevice, // [in]  physical device handle
            &properties);   // [out] physical device properties
        
        // Unique identifier for the Vendor of the device. How to process the 
        // ID depends on the whether the device is installed into PCI slot or
        // not.
        //
        //      PCI: the low sixteen bits contains PCI vendor ID, remaining bits 
        //           are zero. ID is issued either PCI-SIG or Khronos.
        //  Non-PCI: dictated by operating system or platform policies (ARGH)
        uint32_t vendorId = properties.vendorID;
        
        // Unique identifier for the device selected by the Vendor of the 
        // device. The value identifies both the device version and any major 
        // configuration options (e.g. core count). The same device ID should 
        // be used for all physical implementations of that device version and 
        // configuration [previous is from spec].
        uint32_t deviceId = properties.deviceID;
        
        // Physical device type.
        VkPhysicalDeviceType type = properties.deviceType;
        std::string typeDesc =
            vk::stringify::toString(type)      + " (" +
            vk::stringify::toDescription(type) + ")";
        
        // Device name
        std::string deviceName = properties.deviceName;
        
        // Universally unique identifier string
        std::stringstream ss;
        for (int i = 0; i < VK_UUID_SIZE; i++)
        {
            uint8_t u = properties.pipelineCacheUUID[i];
            ss << std::setfill ('0')
               << std::setw(sizeof(uint8_t) * 2)
               << std::hex
               << int(u);

            if (i == 3 || i == 5 || i == 7)
                ss << "-";
        }
        std::string uuidString = ss.str();

        using namespace vk::stringify;
        std::string apiVersionStr    = versionNumber(properties.apiVersion);
        std::string driverVersionStr = versionNumber(properties.driverVersion);

        std::cout << "Physical Device"    << std::endl;
        std::cout << "  Name:           " << deviceName           << std::endl;
        std::cout << "  Type:           " << typeDesc             << std::endl;
        std::cout << "  API version:    " << apiVersionStr        << std::endl;
        std::cout << "  Driver version: " << driverVersionStr     << std::endl;
        std::cout << "  Vendor ID:      " << std::hex << vendorId << std::endl;
        std::cout << "  Device ID:      " << std::hex << deviceId << std::endl;
        std::cout << "  UUID:           " << uuidString           << std::endl;
    }
}

} // namespace vk_test
} // namespace kuu

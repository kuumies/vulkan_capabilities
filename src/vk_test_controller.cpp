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
#include <QtCore/QDebug>
#include <QtCore/QUuid>

namespace kuu
{
namespace vk_test
{

/* -------------------------------------------------------------------------- */

Controller::Controller()
{
    createInstance();
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
void Controller::createInstance()
{
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
    info.sType                   = infoType; // Must be this definition
    info.pNext                   = NULL;     // No extension specific structures
    info.flags                   = 0;        // Must be 0, reserved for future use
    info.pApplicationInfo        = &appInfo; // Application information
    info.enabledLayerCount       = 0;        // Count of requested layers
    info.ppEnabledLayerNames     = NULL;     // Requested layer names
    info.enabledExtensionCount   = 0;        // Count of requested extensions
    info.ppEnabledExtensionNames = NULL;     // Requested extension names

    const VkResult result = vkCreateInstance(
        &info,      // instance create info
        NULL,       // no allocation callback
        &instance); // created instance
        
    if (result == VK_SUCCESS)
        return;
        
    switch(result)
    {
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            std::cerr << __FUNCTION__ 
                      << ": instance creation failed as a "
                      << "host memory allocation failed"
                      << std::endl;
            break;
                    
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            std::cerr << __FUNCTION__ 
                      << ": instance creation failed as a "
                      << "device memory allocation failed"
                      << std::endl;
            break;
            
        case VK_ERROR_INITIALIZATION_FAILED:
            std::cerr << __FUNCTION__ 
                      << ": instance creation failed as "
                      << "initialization of an object could "
                      << "not be completed for implementation-"
                      << "specific reasons"
                      << std::endl;
            break;
            
        case VK_ERROR_LAYER_NOT_PRESENT:
            std::cerr << __FUNCTION__ 
                      << ": instance creation failed as a "
                      << "requested layer is not present or "
                      << "could not be loaded."
                      << std::endl;
            break;
         
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            std::cerr << __FUNCTION__ 
                      << ": instance creation failed as a "
                      << "requested extension is not supported."
                      << std::endl;
            break;
            
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            std::cerr << __FUNCTION__ 
                      << ": instance creation failed as the "
                      << "the requested version of Vulkan is not "
                      << "supported by the driver or is otherwise "
                      << "incompatible for implementation-specific "
                      << "reasons."
                      << std::endl;

        default: break; // should never happen according to spec
    }
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
   
   A physical device is usually a single device in a system. As spect says it
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
        switch(result)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                std::cerr << __FUNCTION__ 
                          << ": physical device enumeration failed as a "
                          << "host memory allocation failed"
                          << std::endl;
                break;
                        
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                std::cerr << __FUNCTION__ 
                          << ": physical device enumeration failed as a "
                          << "device memory allocation failed"
                          << std::endl;
                break;
                
            case VK_ERROR_INITIALIZATION_FAILED:
                std::cerr << __FUNCTION__ 
                          << ": physical device enumeration failed as "
                          << "initialization of an object could "
                          << "not be completed for implementation-"
                          << "specific reasons"
                          << std::endl;
                break;
                
            // This should never happen as the count of  physical
            // devices is asked from the instance.
            case VK_INCOMPLETE:

                std::cerr << __FUNCTION__ 
                          << ": physical device enumeration succeeded "
                          << "but the list does not contain all of the "
                          << "devices"
                          << std::endl;

            default: break; // should never happen according to spec
        }
        
        if (result != VK_INCOMPLETE)
            return;
    }
    
    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(
            physicalDevice, // [in]  physical device handle
            &properties);   // [out] physical device properties
            
        // Vulkan API version supported by the device.
        uint32_t versionMajor = VK_VERSION_MAJOR(properties.apiVersion);
        uint32_t versionMinor = VK_VERSION_MINOR(properties.apiVersion);
        uint32_t versionPatch = VK_VERSION_PATCH(properties.apiVersion);

        std::cout << versionMajor << "." <<
                     versionMinor << "." <<
                     versionPatch << std::endl;
        
        // Vendor-specified version of the driver.
        uint32_t driverVersion = properties.driverVersion;
        
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
        std::string typeDesc;
        switch(type)
        {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                typeDesc += "Other: ";
                typeDesc += "the device does not match any other available "
                            "types.";
                break;
                
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                typeDesc += "Integrated GPU: ";
                typeDesc += "the device is typically one embedded in or "
                            "tightly coupled with the host.";
                break;
                
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                typeDesc += "Discrete GPU: ";
                typeDesc += "the device is typically a separate processor "
                            "connected to the host via an interlink.";
                break;
                
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                typeDesc += "Integrated GPU: ";
                typeDesc += "the device is typically a virtual node in a "
                            "virtualization environment.";
                break;
                
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                typeDesc += "Integrated GPU: ";
                typeDesc += "the device is typically running on the same "
                            "processors as the host.";
                break;
        }
        
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
        std::cout << uuidString << std::endl;
    }
}

} // namespace vk_test
} // namespace kuu

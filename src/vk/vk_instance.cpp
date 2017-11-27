/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Instance class
 * -------------------------------------------------------------------------- */

#include "vk_instance.h"

/* -------------------------------------------------------------------------- */

#include <algorithm>
#include <iostream>

/* -------------------------------------------------------------------------- */

#include "vk_stringify.h"
#include "vk_windows.h"

namespace kuu
{
namespace vk
{
namespace
{

/* -------------------------------------------------------------------------- *
   Validation layer debug callback. Prints the debug info to standard
   error output.
 * -------------------------------------------------------------------------- */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT /*flags*/,
    VkDebugReportObjectTypeEXT /*objType*/,
    uint64_t /*obj*/,
    size_t /*location*/,
    int32_t /*code*/,
    const char* /*layerPrefix*/,
    const char* msg,
    void* /*userData*/) {

    std::cerr << __FUNCTION__
              << ": Vulkan validation layer: "
              << msg
              << std::endl;
    return VK_FALSE;
}

/* -------------------------------------------------------------------------- *
   Returns a vector of instance extension properties.
 * -------------------------------------------------------------------------- */
std::vector<VkExtensionProperties> getExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(
        NULL,            // [in]  NULL -> Vulkan implementation extensions
        &extensionCount, // [out] Extensions count
        NULL);           // [out] NULL -> Get only count

    std::vector<VkExtensionProperties> extensions;
    if (extensionCount)
    {
        extensions.resize(extensionCount);
        vkEnumerateInstanceExtensionProperties(
            nullptr,            // [in]  NULL -> Vulkan implementation extensions
            &extensionCount,    // [in, out] Extensions count
            extensions.data()); // [out] Extensions
    }

    return extensions;
}

/* -------------------------------------------------------------------------- *
   Returns a vector of instance layer properties.
 * -------------------------------------------------------------------------- */
std::vector<VkLayerProperties> getLayers()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(
        &layerCount, // [out] Layer count
        nullptr);    // [out] NULL -> Get only count

    std::vector<VkLayerProperties> layers;
    if (layerCount > 0)
    {
        layers.resize(layerCount);
        vkEnumerateInstanceLayerProperties(
            &layerCount,    // [in, out] Layers count
            layers.data()); // [out]     Layers
    }

    return layers;
}

/* -------------------------------------------------------------------------- *
   Returns a vector of physical devices.
 * -------------------------------------------------------------------------- */
std::vector<PhysicalDevice> getPhysicalDevices(const VkInstance& instance)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(
        instance,    // [in]  Instance handle
        &physicalDeviceCount, // [out] Physical device count
        NULL);                // [in]  Pointer to vector of physical devices, NULL
                              // so the physical device count is returned.

    if (physicalDeviceCount == 0)
    {
        std::cerr << __FUNCTION__
                  << ": no physical devices"
                  << std::endl;
        return std::vector<PhysicalDevice>();
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    VkResult result = vkEnumeratePhysicalDevices(
        instance,                // [in]      Instance handle
        &physicalDeviceCount,    // [in, out] Physical device count
        physicalDevices.data()); // [out]     Pointer to vector of physical devices

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": physical device enumeration failed as "
                  << vk::stringify::resultDesc(result)
                  << std::endl;
        return std::vector<PhysicalDevice>();
    }

    std::vector<PhysicalDevice> out;
    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
        out.push_back(PhysicalDevice(physicalDevice, instance));
    return out;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- *
   Constructs the instance info.
 * -------------------------------------------------------------------------- */
InstanceInfo::InstanceInfo()
{
    extensions = getExtensions();
    layers     = getLayers();
}

/* -------------------------------------------------------------------------- *
   Returns true if the extension is supported.
 * -------------------------------------------------------------------------- */
bool InstanceInfo::isExtensionSupported(const std::string& extension) const
{
    const auto it = std::find_if(
        extensions.begin(),
        extensions.end(),
        [extension](const VkExtensionProperties& ex)
    {
        return std::string(ex.extensionName) ==
               std::string(extension);
    });

    return it != extensions.end();
}

/* -------------------------------------------------------------------------- *
   Returns true if the layer is supported.
 * -------------------------------------------------------------------------- */
bool InstanceInfo::isLayerSupported(const std::string& layer) const
{
    const auto it = std::find_if(
        layers.begin(),
        layers.end(),
        [layer](const VkLayerProperties& l)
    {
        return std::string(l.layerName) ==
               std::string(layer);
    });

    return it != layers.end();
}

/* -------------------------------------------------------------------------- *
   Implementation of Instance class.
 * -------------------------------------------------------------------------- */
struct Instance::Impl
{
    /* ---------------------------------------------------------------------- *
       Destroys the Vulkan instance.
     * ---------------------------------------------------------------------- */
    ~Impl()
    {
        destroy();
    }

    /* ---------------------------------------------------------------------- *
       Creates the Vulkan instance.
     * ---------------------------------------------------------------------- */
    void create()
    {
        if (validate)
        {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            layers.push_back("VK_LAYER_LUNARG_standard_validation");
        }

        std::vector<const char*> extensionNames;
        for (const std::string& extension : extensions)
            extensionNames.push_back(extension.c_str());

        std::vector<const char*> layerNames;
        for (const std::string& layer : layers)
            layerNames.push_back(layer.c_str());

        const uint32_t extensionCount =
            static_cast<uint32_t>(extensionNames.size());
        const uint32_t layerCount =
            static_cast<uint32_t>(layerNames.size());

        VkStructureType appInfoType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        VkApplicationInfo appInfo;
        appInfo.sType              = appInfoType;              // Must be this definition
        appInfo.pNext              = NULL;                     // No extension specific structures
        appInfo.pApplicationName   = applicationName.c_str();  // Name of the application
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // Version of the application
        appInfo.pEngineName        = engineName.c_str();       // Name of the "engine"
        appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0); // Version of the "engine"
        appInfo.apiVersion         = VK_API_VERSION_1_0;       // Version of the Vulkan API

        VkStructureType infoType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        VkInstanceCreateInfo info;
        info.sType                   = infoType;              // Must be this definition
        info.pNext                   = NULL;                  // No extension specific structures
        info.flags                   = 0;                     // Must be 0, reserved for future use
        info.pApplicationInfo        = &appInfo;              // Application information
        info.enabledLayerCount       = layerCount;            // Count of requested layers
        info.ppEnabledLayerNames     = layerNames.data();     // Requested layer names
        info.enabledExtensionCount   = extensionCount;        // Count of requested extensions
        info.ppEnabledExtensionNames = extensionNames.data(); // Requested extension names

        const VkResult result = vkCreateInstance(
            &info,      // [in]  Instance create info
            NULL,       // [in]  No allocation callback
            &instance); // [out] Instance handle

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": instance creation failed as "
                      << vk::stringify::result(result)
                    << std::endl;
            return;
        }

        if (validate)
        {
            VkDebugReportCallbackCreateInfoEXT debugInfo = {};
            debugInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debugInfo.flags        = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            debugInfo.pfnCallback  = debugCallback;

            vk::windows::vkCreateDebugReportCallback(
                instance,
                &debugInfo,
                nullptr,
                &callback);
        }

        physicalDevices = getPhysicalDevices(instance);
    }

    /* ---------------------------------------------------------------------- *
       Destroys the Vulkan instance unless user has manually destroy'd it.
     * ---------------------------------------------------------------------- */
    void destroy()
    {
        for (PhysicalDevice& physicalDevice : physicalDevices)
            physicalDevice.destroy();
        physicalDevices.clear();

        if (callback)
            vk::windows::vkDestroyDebugReportCallback(
                instance,
                callback,
                nullptr);

        vkDestroyInstance(
            instance, // [in] Handle to instance, can be a VK_NULL_HANDLE
            NULL);    // [in] No allocation callback

        instance = VK_NULL_HANDLE;
        callback = nullptr;
    }

    VkInstance instance = VK_NULL_HANDLE;
    VkDebugReportCallbackEXT callback = nullptr;
    InstanceInfo info;
    bool validate = false;
    std::string applicationName;
    std::string engineName;
    std::vector<std::string> extensions;
    std::vector<std::string> layers;
    std::vector<PhysicalDevice> physicalDevices;
};

/* -------------------------------------------------------------------------- *
   Constructs an instance of Instance class.
 * -------------------------------------------------------------------------- */
Instance::Instance()
    : impl(std::make_shared<Impl>())
{}

/* -------------------------------------------------------------------------- *
   Sets an application name. Returns reference to this instance.
 * -------------------------------------------------------------------------- */
Instance &Instance::setApplicationName(const std::string& name)
{ impl->applicationName = name; return *this; }

/* -------------------------------------------------------------------------- *
   Returns the application name or an empty string if no name is set.
 * -------------------------------------------------------------------------- */
std::string Instance::applicationName() const
{ return impl->applicationName; }

/* -------------------------------------------------------------------------- *
   Sets an engine name. Returns reference to this instance.
 * -------------------------------------------------------------------------- */
Instance &Instance::setEngineName(const std::string &name)
{ impl->engineName = name; return *this; }

/* -------------------------------------------------------------------------- *
   Returns the engine name or an empty string if no name is set.
 * -------------------------------------------------------------------------- */
std::string Instance::engineName() const
{ return impl->engineName; }

/* -------------------------------------------------------------------------- *
   Sets a vector of extensions names. Returns reference to this instance.
 * -------------------------------------------------------------------------- */
Instance& Instance::setExtensionNames(
        const std::vector<std::string>& extensionNames)
{ impl->extensions= extensionNames; return *this; }

/* -------------------------------------------------------------------------- *
   Returns the extensions names set by user.
 * -------------------------------------------------------------------------- */
std::vector<std::string> Instance::extensionNames() const
{ return impl->extensions; }

/* -------------------------------------------------------------------------- *
   Sets a vector of layer names. Returns reference to this instance.
 * -------------------------------------------------------------------------- */
Instance& Instance::setLayerNames(
        const std::vector<std::string>& layerNames)
{ impl->layers = layerNames; return *this; }

/* -------------------------------------------------------------------------- *
   Returns the layer names set by user.
 * -------------------------------------------------------------------------- */
std::vector<std::string> Instance::layerNames() const
{ return impl->layers; }

/* -------------------------------------------------------------------------- *
   Sets the validation layer enabled or disabled.
 * -------------------------------------------------------------------------- */
Instance& Instance::setValidateEnabled(bool validate)
{ impl->validate = validate; return *this; }

/* -------------------------------------------------------------------------- *
   Returns the validate layer enabled status.
 * -------------------------------------------------------------------------- */
bool Instance::isValidateEnabled() const
{ return impl->validate; }

/* -------------------------------------------------------------------------- *
   Creates the Vulkan instance. User can create the instance only once.
 * -------------------------------------------------------------------------- */
void Instance::create()
{
    if (!isValid())
        impl->create();
}

/* -------------------------------------------------------------------------- *
   Manually destroys the Vulkan instance.
 * -------------------------------------------------------------------------- */
void Instance::destroy()
{ impl->destroy(); }

/* -------------------------------------------------------------------------- *
   Returns true if the instance handle is not a VK_NULL_HANDLE.
 * -------------------------------------------------------------------------- */
bool Instance::isValid() const
{ return impl->instance != VK_NULL_HANDLE; }

/* -------------------------------------------------------------------------- *
   Returns the handle to instance data. The handle is VK_NULL_HANDLE if the
   instance has not been created or the system does not have a Vulkan imple-
   mentation.
 * -------------------------------------------------------------------------- */
VkInstance Instance::handle() const
{ return impl->instance; }

/* -------------------------------------------------------------------------- *
   Returns the physical devices with Vulkan ability.
 * -------------------------------------------------------------------------- */
std::vector<PhysicalDevice> Instance::physicalDevices() const
{ return impl->physicalDevices; }

/* -------------------------------------------------------------------------- *
   Returns the physical device by index. If the index is not valid then
   an invalid physical device is returned.
 * -------------------------------------------------------------------------- */
PhysicalDevice Instance::physicalDevice(int index) const
{
    if (index < 0 || index >= impl->physicalDevices.size())
        return PhysicalDevice(VK_NULL_HANDLE, VK_NULL_HANDLE);
    return impl->physicalDevices[index];
}

} // namespace vk
} // namespace kuu

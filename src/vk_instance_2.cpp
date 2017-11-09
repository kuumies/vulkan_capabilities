/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Instance class
 * ---------------------------------------------------------------- */
 
#include "vk_isntance_2.h"

/* ---------------------------------------------------------------- */

#include "vk_physical_device.h"

namespace kuu
{
namespace vk
{
namespace
{

/* ---------------------------------------------------------------- *
   Validation layer debug callback.
 * ---------------------------------------------------------------- */
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

/* ---------------------------------------------------------------- */
    
std::vector<const char*> toConstCharVector(
    const std::vector<std::string>& strings)
{
    std::vector<const char*> out;
    for (const std::string& string : strings)
        out.push_back(string.c_str());
    return out;
}
    
} // anonymous namespace

/* ---------------------------------------------------------------- */

struct Instance::Data
{
    std::string applicationName;
    std::string engineName;
    std::vector<std::string> extensions;
    std::vector<std::string> layers;
    
    VkInstance instance;
    bool created;
    
    std::vector<PhysicalDevice> physicalDevices;
};

/* ---------------------------------------------------------------- */

Instance& Instance::setApplicationName(const std::string& applicationName)
{
    d->applicationName = applicationName;
    return *this;
}

/* ---------------------------------------------------------------- */

Instance& Instance::setEngineName(const std::string& engineName)
{
    d->engineName = engineName;
    return *this;
}
    
/* ---------------------------------------------------------------- */

Instance& Instance::setExtensions(std::vector<std::string>& extensions)
{
    d->extensions = extensions;
    return *this;
}

/* ---------------------------------------------------------------- */

Instance& Instance::setLayers(std::vector<std::string>& layers)
{
    d->layers = layers;
    return *this;
}
    
/* ---------------------------------------------------------------- */

bool Instance::create()
{
    //////////////////////////////////////////////////////////////////
    // 1) create the instance
    
    std::vector<const char*> extensions = toConstCharVector(d->extensions);
    std::vector<const char*> layers     = toConstCharVector(d->layers);
    
    VkApplicationInfo appInfo = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = d->applicationName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = d->engineName.c_str();
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo        = &appInfo;
    instanceInfo.enabledLayerCount       = uint32_t(layers.size());
    instanceInfo.enabledExtensionCount   = uint32_t(extensions.size());
    
    if (extensions.size())
        instanceInfo.ppEnabledExtensionNames = extensions.data();
    if (layers.size())
        instanceInfo.ppEnabledLayerNames = layers.data();

    const VkResult result = vkCreateInstance(
        &instanceInfo,
        nullptr,
        &d->instance);
    
    //////////////////////////////////////////////////////////////////
    // 2) create the physical devices
    
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(
        d->instance,
        &physicalDeviceCount,
        nullptr);

    if (physicalDeviceCount > 0)
    {
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(
            d->instance,
            &physicalDeviceCount,
            physicalDevices.data());

        for (VkPhysicalDevice device : physicalDevices)
            d->physicalDevices.push_back(PhysicalDevice(device));
    }

    //////////////////////////////////////////////////////////////////
    // 3) create the debug callback if user has set the debug report
    //    extensions and it is supported by system.
    if (containsExtension(d->extensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) &&
        isExtensionSupported(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
    {
        VkDebugReportCallbackCreateInfoEXT debugInfo = {};
        debugInfo.sType        = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugInfo.flags        = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
        debugInfo.pfnCallback  = debugCallback;
    }
    
    d->created = true;
    return result == VK_SUCCESS &&
           d->physicalDevices.size() > 0;
}

/* ---------------------------------------------------------------- */

bool Instance::destroy()
{
    if (!d->created)
        return false;
    vkDestroyInstance(d->instance, nullptr);
    return true;
}

/* ---------------------------------------------------------------- */

vkInstance Instance::handle() const
{ return d->instance; }

/* ---------------------------------------------------------------- */

std::vector<PhysicalDevice> Instance::physicalDevices() const
{ return d->physicalDevices; }

/* ---------------------------------------------------------------- */

Instance& Instance::get()
{
    static Instance instance;
    return instance;
}

/* ---------------------------------------------------------------- */

bool Instance::isExtensionSupported(const std::string& extension)
{
    // Get the extensions count.
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           nullptr);
    if (extensionCount == 0)
        return false;

    // Get the extensions.
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr,
                                           &extensionCount,
                                           extensions.data());

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

/* ---------------------------------------------------------------- */

bool Instance::isLayerSupported(const std::string& layer)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    if (layerCount == 0)
        return false;

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount,
                                       layers.data());

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
    
/* ---------------------------------------------------------------- */

Instance::Instance()
    : d(std::make_shared<Data>())
{}

} // namespace vk
} // namespace kuu

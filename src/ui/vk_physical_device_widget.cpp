/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::PhysicalDeviceWidget class
 * -------------------------------------------------------------------------- */

#include "vk_physical_device_widget.h"
#include "ui_vk_physical_device_widget.h"

/* -------------------------------------------------------------------------- */

#include <iostream>
#include <sstream>

/* -------------------------------------------------------------------------- */

#include "../vk/vk_stringify.h"
#include "../vk/vk_windows.h"

namespace kuu
{
namespace vk_test
{

/* -------------------------------------------------------------------------- */

struct PhysicalDeviceWidget::Data
{
    Data(PhysicalDeviceWidget* self)
    {
        ui.setupUi(self);
    }
    Ui::PhysicalDeviceWidget ui;
};

/* -------------------------------------------------------------------------- */

PhysicalDeviceWidget::PhysicalDeviceWidget(QWidget* parent)
    : QWidget(parent)
    , d(std::make_shared<Data>(this))
{}

/* -------------------------------------------------------------------------- */

void PhysicalDeviceWidget::setPhysicalDevice(
    const VkInstance& instance,
    const VkPhysicalDevice& physicalDevice)
{
    using namespace vk;

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

    using namespace vk::stringify;
    std::string uuidString       = toString(properties.pipelineCacheUUID);
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

    d->ui.apiVersion->setText(QString::fromStdString(apiVersionStr));
    d->ui.driverVersion->setText(QString::fromStdString(driverVersionStr));

    std::stringstream ss;
    ss << std::hex << vendorId;
    d->ui.vendorId->setText(QString::fromStdString(ss.str()));

    ss.str("");
    ss.clear();
    ss << std::hex << deviceId;

    d->ui.deviceId->setText(QString::fromStdString(ss.str()));
    d->ui.uuid->setText(QString::fromStdString(uuidString));

    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,            // [in]  physical device handle
        &queueFamilyPropertyCount, // [out] queue family property count
        NULL);                     // [in]  properties, NULL to get count

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(
        queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice,                // [in]  physical device handle
        &queueFamilyPropertyCount,     // [in]  queue family property count
        queueFamilyProperties.data()); // [out] queue family properties

    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyPropertyCount; ++queueFamilyIndex)
    {
        const VkQueueFamilyProperties& property = queueFamilyProperties[queueFamilyIndex];

        std::cout << "Queue Family Index: " << queueFamilyIndex << std::endl;
        std::cout << "Queue count: " << property.queueCount  << std::endl;

        std::string capabilitiesStr = toString(property.queueFlags);
        std::string minImageTransferGranularityStr = toString(property.minImageTransferGranularity);
        std::cout << "Capabilities: " << capabilitiesStr << std::endl;
        std::cout << "Count of valid time stamp bits: " << property.timestampValidBits << std::endl;
        std::cout << "Minimum image transfer granularity: " << minImageTransferGranularityStr << std::endl;

#ifdef _WIN32
        using namespace vk::windows;
        const VkBool32 result =
            vkGetPhysicalDeviceWin32PresentationSupportKHR(
                instance,
                physicalDevice,    // [in] physical device handle
                queueFamilyIndex); // [in] queue family index
        std::cout << "Presentation support: " << (result == VK_TRUE ? "yes" : "no") << std::endl;
#endif
    }
}

} // namespace vk_test
} // namespace kuu

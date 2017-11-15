/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_test::PhysicalDeviceSelectionDialog class
 * -------------------------------------------------------------------------- */

#include "vk_test_physical_device_selection_dialog.h"
#include "ui_vk_test_physical_device_selection_dialog.h"

/* -------------------------------------------------------------------------- */

#include <iostream>

/* -------------------------------------------------------------------------- */

#include "../vk/vk_stringify.h"
#include "../vk/vk_windows.h"

namespace kuu
{
namespace vk_test
{
namespace
{

/* -------------------------------------------------------------------------- *
   Enumerates the physical devices.

   A physical device is usually a single device in a system. As spec says it
   could be made up of several individual hardware devices working together.
   I guess like when using SLI mode?

   See: https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#devsandqueues-physical-device-enumeration
 * -------------------------------------------------------------------------- */
std::vector<VkPhysicalDevice> enumeratePhysicalDevices(const VkInstance& instance)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(
        instance,             // [in]  Instance handle
        &physicalDeviceCount, // [out] Physical device count
        NULL);                // [in]  Pointer to vector of physical devices, NULL
                              // so the physical device count is returned.

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    const VkResult result = vkEnumeratePhysicalDevices(
        instance,                // [in]      Instance handle
        &physicalDeviceCount,    // [in, out] Physical device count
        physicalDevices.data()); // [out]     Pointer to vector of physical devices

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": physical device enumeration failed as "
                  << vk::stringify::toDescription(result)
                  << std::endl;

        if (result != VK_INCOMPLETE)
            return std::vector<VkPhysicalDevice>();
    }

    return physicalDevices;
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct PhysicalDeviceSelectionDialog::Data
{
    Data(PhysicalDeviceSelectionDialog* self)
    {
        ui.setupUi(self);
        QObject::connect(
            ui.comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            self, &PhysicalDeviceSelectionDialog::onPhysicalDeviceSelectionChanged);
    }
    Ui::PhysicalDeviceSelectionDialog ui;
    VkInstance instance;
    std::vector<VkPhysicalDevice> physicalDevices;
};

/* -------------------------------------------------------------------------- */

PhysicalDeviceSelectionDialog::PhysicalDeviceSelectionDialog(QWidget* parent)
    : QDialog(parent)
    , d(std::make_shared<Data>(this))
{}

/* -------------------------------------------------------------------------- */

void PhysicalDeviceSelectionDialog::setInstance(const VkInstance& instance)
{
    d->instance        = instance;
    d->physicalDevices = enumeratePhysicalDevices(instance);

    for (const VkPhysicalDevice& physicalDevice : d->physicalDevices)
    {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(
            physicalDevice, // [in]  physical device handle
            &properties);   // [out] physical device properties

        // Device name
        const std::string& deviceName = properties.deviceName;
        d->ui.comboBox->blockSignals(true);
        d->ui.comboBox->addItem(QString::fromStdString(deviceName));
        d->ui.comboBox->setCurrentIndex(0);
        d->ui.comboBox->blockSignals(false);
    }

    if (d->physicalDevices.size() > 0)
        onPhysicalDeviceSelectionChanged(0);
}

/* -------------------------------------------------------------------------- */

void PhysicalDeviceSelectionDialog::onPhysicalDeviceSelectionChanged(int index)
{
    if (index == -1)
        return;

    d->ui.physicalDeviceWidget->setPhysicalDevice(
        d->instance,
        d->physicalDevices[index]);
}

} // namespace vk_test
} // namespace kuu

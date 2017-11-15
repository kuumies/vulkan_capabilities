/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_test::PhysicalDeviceSelectionDialog class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <QtWidgets/QDialog>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk_test
{

/* -------------------------------------------------------------------------- *
   A dialog to select the physical Vulkan device.
 * -------------------------------------------------------------------------- */
class PhysicalDeviceSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    // Constructs the dialog with an optional parent.
    explicit PhysicalDeviceSelectionDialog(QWidget* parent = nullptr);

    // Set the instance where to retrieve the physical devices.
    void setInstance(const VkInstance& instance);

private slots:
    void onPhysicalDeviceSelectionChanged(int index);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk_test
} // namespace kuu

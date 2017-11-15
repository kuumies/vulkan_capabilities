/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::PhysicalDeviceWidget class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <QtWidgets/QWidget>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk_test
{

/* -------------------------------------------------------------------------- *
   A widget to show physical Vulkan device properties.
 * -------------------------------------------------------------------------- */
class PhysicalDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    // Constructs the widget with an optional parent.
    explicit PhysicalDeviceWidget(QWidget* parent = nullptr);

    // Sets the physical device along with an instance to get more
    // information about device capabilities.
    void setPhysicalDevice(const VkInstance& instance,
                           const VkPhysicalDevice& physicalDevice);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk_test
} // namespace kuu

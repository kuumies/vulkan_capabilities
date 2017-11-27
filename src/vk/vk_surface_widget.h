/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::Widget class
 * -------------------------------------------------------------------------- */

#pragma once

#include <memory>
#include <QtWidgets/QWidget>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   A widget to present a Vulkan-framebuffer.

   Note that at the moment only Windows OS is supported.

   The surface is destroyd when the widget goes out-of-scope. This
   needs to happen before Vulkan instance is destroyed.
 * -------------------------------------------------------------------------- */
class SurfaceWidget : public QWidget
{
public:
    // Constructs the widget and creates the Vulkan surface. Use
    // isValid() to know whether the surface creation succeeded.
    // The given in instance needs to be valid.
    SurfaceWidget(const VkInstance& instance, QWidget* parent = nullptr);

    // Returns true if the surface handle is not a null handle.
    bool isValid() const;

    // Returns the surface handle. Handle is a VK_NULL_HANDLE
    // if the surface creation has failed.
    VkSurfaceKHR surface() const;

protected:
    // Paint event needs to be disabled.
    void paintEvent(QPaintEvent* e);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu

/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::SurfaceWidget class
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
    Q_OBJECT

public:
    // Constructs the widget and creates the Vulkan surface. Use
    // isValid() to know whether the surface creation succeeded.
    // The given in instance needs to be valid.
    SurfaceWidget(QWidget* parent = nullptr);

    // Sets the Vulkan instance.
    SurfaceWidget& setInstance(const VkInstance& instance);
    VkInstance instance() const;

    // Creates and distroys the Vulkan surface.
    void createSurface();
    void destroySurface();

    // Returns true if the surface handle is not a null handle.
    bool isValid() const;

    // Returns the surface handle. Handle is a VK_NULL_HANDLE
    // if the surface creation has failed.
    VkSurfaceKHR handle() const;

signals:
    void interval();
    void resized();

protected:
    // Paint event needs to be disabled.
    void paintEvent(QPaintEvent* e);
    void timerEvent(QTimerEvent* e);
    void resizeEvent(QResizeEvent* e);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu

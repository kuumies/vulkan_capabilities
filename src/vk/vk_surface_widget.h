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
    void wheel(int delta);
    void mouseMove(const QPoint& pos, int buttons, int modifiers);
    void key(int key, int modifiers, bool down);

protected:
    // Paint event needs to be disabled.
    void paintEvent(QPaintEvent* e) override;
    void timerEvent(QTimerEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk
} // namespace kuu

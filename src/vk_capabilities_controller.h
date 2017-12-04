/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <QtCore/QObject>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- *
   A controller for Vulkan Capabilities application.
 * -------------------------------------------------------------------------- */
class Controller : public QObject
{
    Q_OBJECT

public:
    // Constructs the controller. UI is not visible until showUi is called.
    // This will create the Vulkan instance, enumerates the physical devices
    // and creates the model for UI to display the capabilities data.
    Controller();

    // Starts the application.
    void start();

public:
    void runDeviceTest(int deviceIndex);

    void onSurfaceInterval();
    void onSurfaceResized();
    void onSurfaceWheel(int delta);
    void onSurfaceMouseMove(const QPoint& offset);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk_capabilities
} // namespace kuu

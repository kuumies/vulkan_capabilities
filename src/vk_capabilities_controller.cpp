/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_capabilities_controller.h"

/* -------------------------------------------------------------------------- */

#include <QtWidgets/QApplication>
#include <future>

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_data_creator.h"
#include "vk_capabilities_main_window.h"
#include "vk_instance.h"
#include "vk_surface_widget.h"

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- */

struct Controller::Impl
{
    // Main window
    std::unique_ptr<MainWindow> mainWindow;
    // Vulkan objects
    std::shared_ptr<vk::Instance> instance;
    // Surface widget
    std::unique_ptr<vk::SurfaceWidget> surfaceWidget;
    // Capabilities data created from vulkan objects.
    std::shared_ptr<Data> capabilitiesData;
};

/* -------------------------------------------------------------------------- */

Controller::Controller()
    : impl(std::make_shared<Impl>())
{}

/* -------------------------------------------------------------------------- */

void Controller::start()
{

    impl->mainWindow = std::unique_ptr<MainWindow>(new MainWindow());
    impl->mainWindow->setEnabled(false);
    impl->mainWindow->show();
    impl->mainWindow->showProgress();

    std::future<void> vulkanInstanceTask = std::async([&]()
    {
        std::vector<std::string> extensions;
        if (vk::Instance::isExtensionSupported("VK_KHR_get_physical_device_properties2"))
            extensions.push_back("VK_KHR_get_physical_device_properties2");
#ifdef _WIN32
        if (vk::Instance::isExtensionSupported("VK_KHR_win32_surface"))
            extensions.push_back("VK_KHR_win32_surface");
#endif
        impl->instance         = std::make_shared<vk::Instance>(extensions);
    });

    while (vulkanInstanceTask.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    impl->surfaceWidget = std::unique_ptr<vk::SurfaceWidget>(new vk::SurfaceWidget(impl->instance->instance));

    std::future<void> uiDataTask = std::async([&]()
    {
        impl->capabilitiesData = DataCreator(*impl->instance, *impl->surfaceWidget).data();
        impl->mainWindow->setDataAsync(impl->capabilitiesData);
    });

    while (uiDataTask.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    impl->mainWindow->setEnabled(true);
    impl->mainWindow->update();
    impl->mainWindow->hideProgress();
}

} // namespace vk_capabilities
} // namespace kuu

/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_capabilities_controller.h"

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_data_creator.h"
#include "vk_capabilities_main_window.h"
#include "vk_instance.h"


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
    // Capabilities data created from vulkan objects.
    std::shared_ptr<Data> capabilitiesData;
};

/* -------------------------------------------------------------------------- */

Controller::Controller()
    : impl(std::make_shared<Impl>())
{
    std::vector<std::string> extensions;
    if (vk::Instance::isExtensionSupported(
            "VK_KHR_get_physical_device_properties2"))
    {
        extensions.push_back("VK_KHR_get_physical_device_properties2");
    }

    impl->instance = std::make_shared<vk::Instance>(extensions);
    impl->capabilitiesData = DataCreator(*impl->instance).data();
}

/* -------------------------------------------------------------------------- */

void Controller::showUi()
{
    impl->mainWindow = std::unique_ptr<MainWindow>(new MainWindow());
    impl->mainWindow->setData(impl->capabilitiesData);
    impl->mainWindow->show();
}

} // namespace vk_capabilities
} // namespace kuu

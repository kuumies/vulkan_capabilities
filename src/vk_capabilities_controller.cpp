/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::Controller class
 * -------------------------------------------------------------------------- */
 
#include "vk_capabilities_controller.h"

/* -------------------------------------------------------------------------- */

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenuBar>
#include <future>

/* -------------------------------------------------------------------------- */

#include "vk/vk_helper.h"
#include "vk/vk_instance.h"
#include "vk/vk_surface_properties.h"
#include "vk/vk_surface_widget.h"
#include "vk/vk_swapchain.h"
#include "vk_capabilities_data_creator.h"
#include "vk_capabilities_main_window.h"

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
    std::unique_ptr<vk::SurfaceWidget> surfaceWidget;
    std::vector<vk::SurfaceProperties> surfaceProperties;

    // Capabilities data created from vulkan objects.
    std::shared_ptr<Data> capabilitiesData;

    // Test data.
    std::shared_ptr<vk::Swapchain> swapChain;
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
    connect(impl->mainWindow.get(), &MainWindow::runDeviceTest,
            this, &Controller::runDeviceTest);

    std::future<void> vulkanInstanceTask = std::async([&]()
    {
        const vk::InstanceInfo instanceInfo;

        std::vector<std::string> extensions;
        if (instanceInfo.isExtensionSupported("VK_KHR_get_physical_device_properties2"))
            extensions.push_back("VK_KHR_get_physical_device_properties2");
        if (instanceInfo.isExtensionSupported("VK_KHR_display"))
            extensions.push_back("VK_KHR_display");
        if (instanceInfo.isExtensionSupported("VK_KHR_surface"))
        {
            extensions.push_back("VK_KHR_surface");
#ifdef _WIN32
            if (instanceInfo.isExtensionSupported("VK_KHR_win32_surface"))
                extensions.push_back("VK_KHR_win32_surface");
#endif
        }
        impl->instance = std::make_shared<vk::Instance>();
        impl->instance->setApplicationName("V-Caps");
        impl->instance->setEngineName("V-CAPS-ENGINE");
        impl->instance->setExtensionNames(extensions);
        impl->instance->setValidateEnabled(true);
        impl->instance->create();
    });

    while (vulkanInstanceTask.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    impl->surfaceWidget = std::unique_ptr<vk::SurfaceWidget>(new vk::SurfaceWidget(impl->instance->handle()));
    impl->surfaceWidget->resize(720, 576);

    std::future<void> uiDataTask = std::async([&]()
    {
        auto devices = impl->instance->physicalDevices();
        for (vk::PhysicalDevice& device : devices)
        {
            impl->surfaceProperties.push_back(
                vk::SurfaceProperties(
                    impl->instance->handle(),
                    device.physicalDeviceHandle(),
                    impl->surfaceWidget->surface()));
        }
        impl->capabilitiesData = DataCreator(*impl->instance, *impl->surfaceWidget, impl->surfaceProperties).data();
        impl->mainWindow->setDataAsync(impl->capabilitiesData);
    });

    while (uiDataTask.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);


    impl->mainWindow->update();
    impl->mainWindow->hideProgress();
    if (impl->capabilitiesData->hasVulkan &&
        impl->capabilitiesData->physicalDeviceData.size())
    {
        impl->mainWindow->setEnabled(true);
    }
}

/* -------------------------------------------------------------------------- */

void Controller::runDeviceTest(int deviceIndex)
{
    vk::PhysicalDevice physicalDevice =
        impl->instance->physicalDevice(deviceIndex);
    const vk::PhysicalDeviceInfo info = physicalDevice.info();

    const int graphics     = vk::helper::findQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, info.queueFamilies);
    const int presentation = vk::helper::findPresentationQueueFamilyIndex(
        physicalDevice.physicalDeviceHandle(),
        impl->surfaceWidget->surface(),
        info.queueFamilies,
        { graphics });

    physicalDevice.setExtensions( { VK_KHR_SWAPCHAIN_EXTENSION_NAME });
    physicalDevice.addQueueFamily(graphics,     1, 1.0f);
    physicalDevice.addQueueFamily(presentation, 1, 1.0f);
    physicalDevice.create();
    if (!physicalDevice.isValid())
        return;

    const VkExtent2D widgetExtent = { uint32_t(impl->surfaceWidget->width()), uint32_t(impl->surfaceWidget->height()) };
    const vk::SurfaceProperties surfaceInfo = impl->surfaceProperties[deviceIndex];
    const VkSurfaceFormatKHR surfaceFormat = vk::helper::findSwapchainSurfaceFormat(surfaceInfo.surfaceFormats);
    const VkPresentModeKHR presentMode     = vk::helper::findSwapchainPresentMode(surfaceInfo.presentModes);
    const VkExtent2D extent                = vk::helper::findSwapchainImageExtent(surfaceInfo.surfaceCapabilities, widgetExtent);
    const int imageCount                   = vk::helper::findSwapchainImageCount(surfaceInfo.surfaceCapabilities);

    VkRenderPass renderPass = VK_NULL_HANDLE;
    impl->swapChain = std::make_shared<vk::Swapchain>(impl->surfaceWidget->surface(), physicalDevice.logicalDeviceHandle(), renderPass);
    impl->swapChain->setSurfaceFormat(surfaceFormat)
                    .setPresentMode(presentMode)
                    .setImageExtent(extent)
                    .setImageCount(imageCount)
                    .setPreTransform(surfaceInfo.surfaceCapabilities.currentTransform)
                    .setQueueIndicies( { uint32_t(graphics), uint32_t(presentation) } );
    impl->swapChain->create();
    if (!impl->swapChain->isValid())
        return;
}

} // namespace vk_capabilities
} // namespace kuu

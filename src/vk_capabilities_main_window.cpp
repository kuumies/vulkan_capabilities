/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::MainWindow class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_main_window.h"
#include "ui_vk_capabilities_main_window.h"

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_data.h"

namespace kuu
{
namespace vk_capabilities
{
namespace
{

/* -------------------------------------------------------------------------- *
   Update the UI from the data.
 * -------------------------------------------------------------------------- */

void updateUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    QVBoxLayout* propertiesLayout = new QVBoxLayout();
    QWidget* properties = ui.properties;
    delete properties->layout();
    properties->setLayout(propertiesLayout);

    if (!d.hasVulkan)
    {
        ui.errorMessage->setText("No Vulkan Implementation Found");
        ui.mainStackedWidget->setCurrentIndex(1);
        return;
    }

    if (!d.physicalDeviceData.size())
    {
        ui.errorMessage->setText("No Devices with Vulkan capability");
        ui.mainStackedWidget->setCurrentIndex(1);
        return; // No devices
    }

    ui.mainStackedWidget->setCurrentIndex(0);

    if (deviceIndex == -1)
        deviceIndex = 0;


    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const std::pair<std::string, std::string>& v : dev.mainProperties)
    {
        QLabel* name  = new QLabel(QString::fromStdString(v.first));
        name->setProperty("nameValueLabel", true);
        QLabel* value = new QLabel(QString::fromStdString(v.second));
        value->setProperty("nameValueLabel", true);
        value->setFrameShape(QFrame::Panel);
        value->setFrameShadow(QFrame::Sunken);

        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(name);
        l->addWidget(value);
        propertiesLayout->addLayout(l);
    }

    QMenu* deviceMenu = new QMenu();
    deviceMenu->setFont(QFont("Segoe UI", 10));
    for (const Data::PhysicalDeviceData& dev : d.physicalDeviceData)
        deviceMenu->addAction(QString::fromStdString(dev.name()));

    ui.deviceButton->blockSignals(true);
    ui.deviceButton->setMenu(deviceMenu);
    ui.deviceButton->setText(QString::fromStdString(dev.name()));
    ui.deviceButton->blockSignals(false);
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct MainWindow::Impl
{
    Impl(MainWindow* self)
    {
        ui.setupUi(self);

        const std::map<QToolButton*, int> buttons =
        {
            { ui.mainButton,       0 },
            { ui.extensionsButton, 1 },
            { ui.layersButton,     2 },
            { ui.featuresButton,   3 },
            { ui.memoryButton,     4 },
            { ui.queuesButton,     5 },
            { ui.limitsButton,     6 },
        };

        for (auto button : buttons)
        {
            connect(button.first, &QToolButton::clicked, [&, button]()
            { ui.capabilitiesStackedWidget->setCurrentIndex(button.second); });
        }

    }

    // UI controls
    Ui::MainWindow ui;
    // Data model.
    std::shared_ptr<Data> data;
};

/* -------------------------------------------------------------------------- */

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , impl(std::make_shared<Impl>(this))
{}

/* -------------------------------------------------------------------------- */

void MainWindow::setData(std::shared_ptr<Data> data)
{
    impl->data = data;
    updateUi(impl->ui, *impl->data);
}

} // namespace vk_capabilities
} // namespace kuu

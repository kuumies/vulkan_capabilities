/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::MainWindow class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_main_window.h"
#include "ui_vk_capabilities_main_window.h"

/* -------------------------------------------------------------------------- */

#include <QtWidgets/QCheckBox>

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_data.h"

namespace kuu
{
namespace vk_capabilities
{
namespace
{

/* -------------------------------------------------------------------------- *
   Update the extensions UI from the data.
 * -------------------------------------------------------------------------- */
void updateExtensionsUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    QWidget* widget = ui.extensions;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    QLabel* nameHeader1  = new QLabel("Supported Instance Extension");
    nameHeader1->setProperty("nameHeaderLabel", true);
    QLabel* valueHeader1 = new QLabel("Version");
    valueHeader1->setProperty("nameHeaderLabel", true);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(nameHeader1);
    l->addWidget(valueHeader1);
    layout->addLayout(l);

    for (const std::pair<std::string, std::string>& v : d.instanceExtensions)
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
        layout->addLayout(l);
    }

    QLabel* nameHeader2  = new QLabel("Supported Device Extension");
    nameHeader2->setProperty("nameHeaderLabel", true);
    QLabel* valueHeader2 = new QLabel("Version");
    valueHeader2->setProperty("nameHeaderLabel", true);

    QHBoxLayout* l2 = new QHBoxLayout();
    l2->addWidget(nameHeader2);
    l2->addWidget(valueHeader2);
    layout->addLayout(l2);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const std::pair<std::string, std::string>& v : dev.extensions)
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
        layout->addLayout(l);
    }
}

/* -------------------------------------------------------------------------- *
   Update the layers UI from the data.
 * -------------------------------------------------------------------------- */
void updateLayersUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    QWidget* widget = ui.layers;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    const int SpecMaxWidth = 80;
    const int ImplMaxWidth = 80;

    QLabel* nameHeader  = new QLabel("Supported Layer");
    nameHeader->setProperty("nameHeaderLabel", true);
    QLabel* descHeader  = new QLabel("Description");
    descHeader->setProperty("nameHeaderLabel", true);
    QLabel* specVersionHeader = new QLabel("Spec Version");
    specVersionHeader->setProperty("nameHeaderLabel", true);
    specVersionHeader->setMaximumWidth(SpecMaxWidth);
    QLabel* implVersionHeader = new QLabel("Impl Version");
    implVersionHeader->setProperty("nameHeaderLabel", true);
    implVersionHeader->setMaximumWidth(ImplMaxWidth);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(nameHeader);
    l->addWidget(descHeader);
    l->addWidget(specVersionHeader);
    l->addWidget(implVersionHeader);
    layout->addLayout(l);

    for (const Data::Layer& layer : d.instanceLayers)
    {
        QLabel* name  = new QLabel(QString::fromStdString(layer.name));
        name->setProperty("nameValueLabel", true);
        name->setToolTip(QString::fromStdString(layer.desc));

        QLabel* desc = new QLabel(QString::fromStdString(layer.desc));
        desc->setProperty("nameValueLabel", true);
        desc->setFrameShape(QFrame::Panel);
        desc->setFrameShadow(QFrame::Sunken);

        QLabel* specVer = new QLabel(QString::fromStdString(layer.specVersion));
        specVer->setProperty("nameValueLabel", true);
        specVer->setFrameShape(QFrame::Panel);
        specVer->setFrameShadow(QFrame::Sunken);
        specVer->setMaximumWidth(SpecMaxWidth);

        QLabel* implVer = new QLabel(QString::fromStdString(layer.implVersion));
        implVer->setProperty("nameValueLabel", true);
        implVer->setFrameShape(QFrame::Panel);
        implVer->setFrameShadow(QFrame::Sunken);
        implVer->setMaximumWidth(ImplMaxWidth);

        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(name);
//        l->addWidget(desc);
        l->addWidget(specVer);
        l->addWidget(implVer);
        layout->addLayout(l);
    }
}


/* -------------------------------------------------------------------------- *
   Update the limits UI from the data.
 * -------------------------------------------------------------------------- */
void updateLimitsUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    QWidget* widget = ui.limits;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    QLabel* nameHeader  = new QLabel("Limit");
    nameHeader->setProperty("nameHeaderLabel", true);
    QLabel* valueHeader = new QLabel("Value");
    valueHeader->setProperty("nameHeaderLabel", true);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(nameHeader);
    l->addWidget(valueHeader);
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const Data::Limit& limit : dev.limits)
    {
        QLabel* name  = new QLabel(QString::fromStdString(limit.name));
        name->setProperty("nameValueLabel", true);
        name->setToolTip(QString::fromStdString(limit.desc));

        QLabel* value = new QLabel(QString::fromStdString(limit.value));
        value->setProperty("nameValueLabel", true);
        value->setFrameShape(QFrame::Panel);
        value->setFrameShadow(QFrame::Sunken);

        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(name);
        l->addWidget(value);
        layout->addLayout(l);
    }
}

/* -------------------------------------------------------------------------- *
   Update the UI from the data.
 * -------------------------------------------------------------------------- */
void updateUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    QVBoxLayout* propertiesLayout = new QVBoxLayout();
    QWidget* properties = ui.properties;
    delete properties->layout();
    properties->setLayout(propertiesLayout);

    QVBoxLayout* featuresLayout = new QVBoxLayout();
    QWidget* features = ui.features;
    delete features->layout();
    features->setLayout(featuresLayout);

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
        value->setToolTip("version of Vulkan supported by the device");

        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(name);
        l->addWidget(value);
        propertiesLayout->addLayout(l);
    }
    for (const std::pair<std::string, bool>& v : dev.mainFeatures)
    {
        QLabel* name  = new QLabel(QString::fromStdString(v.first));
        name->setProperty("nameValueLabel", true);

        QLabel* value = new QLabel();
        value->setFrameShape(QFrame::Panel);
        value->setFrameShadow(QFrame::Sunken);
        if (v.second)
        {
            value->setText("Supported");
            value->setProperty("nameValueValidLabel", true);
        }
        else
        {
            value->setText("Unsupported");
            value->setProperty("nameValueInvalidLabel", true);
        }


        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(name);
        l->addWidget(value);
        featuresLayout->addLayout(l);
    }

    updateExtensionsUi(ui, d, deviceIndex);
    updateLayersUi(ui, d, deviceIndex);
    updateLimitsUi(ui, d, deviceIndex);

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

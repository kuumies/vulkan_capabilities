/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::MainWindow class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_main_window.h"
#include "ui_vk_capabilities_main_window.h"

/* -------------------------------------------------------------------------- */

#include <QtWidgets/QCheckBox>

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_about_dialog.h"
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

    const int SpecMaxWidth = 180;
    const int ImplMaxWidth = 180;

    QLabel* nameHeader  = new QLabel("Supported Layer");
    nameHeader->setProperty("nameHeaderLabel", true);
    QLabel* specVersionHeader = new QLabel("Spec Version");
    specVersionHeader->setProperty("nameHeaderLabel", true);
    specVersionHeader->setMaximumWidth(SpecMaxWidth);
    QLabel* implVersionHeader = new QLabel("Impl Version");
    implVersionHeader->setProperty("nameHeaderLabel", true);
    implVersionHeader->setMaximumWidth(ImplMaxWidth);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(nameHeader);
    l->addWidget(specVersionHeader);
    l->addWidget(implVersionHeader);
    layout->addLayout(l);

    for (const Data::Layer& layer : d.instanceLayers)
    {
        QLabel* name  = new QLabel(QString::fromStdString(layer.name));
        name->setProperty("nameValueLabel", true);
        name->setToolTip(QString::fromStdString(layer.desc));

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
   Update the queues UI from the data.
 * -------------------------------------------------------------------------- */
void updateQueuesUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    QWidget* widget = ui.queues;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    const int FamilyIndexMaxWidth = 80;
    const int CountMaxWidth       = 80;
    const int GranularityMaxWidth = 120;
    const int TimestampMaxWidth   = 100;

    QLabel* familyIndexHeader  = new QLabel("Family Index");
    familyIndexHeader->setProperty("nameHeaderLabel", true);
    familyIndexHeader->setMinimumWidth(FamilyIndexMaxWidth);
    familyIndexHeader->setMaximumWidth(FamilyIndexMaxWidth);

    QLabel* countHeader = new QLabel("Queue Count");
    countHeader->setProperty("nameHeaderLabel", true);
    countHeader->setMinimumWidth(CountMaxWidth);
    countHeader->setMaximumWidth(CountMaxWidth);

    QLabel* capabilitiesHeader = new QLabel("Capabilities");
    capabilitiesHeader->setProperty("nameHeaderLabel", true);

    QLabel* granularityHeader = new QLabel("Min Image Transfer\nGranularity");
    granularityHeader->setProperty("nameHeaderLabel", true);
    granularityHeader->setMinimumWidth(GranularityMaxWidth);
    granularityHeader->setMaximumWidth(GranularityMaxWidth);

    QLabel* timestampHeader = new QLabel("Timestamp Valid\nBits");
    timestampHeader->setProperty("nameHeaderLabel", true);
    timestampHeader->setMinimumWidth(TimestampMaxWidth);
    timestampHeader->setMaximumWidth(TimestampMaxWidth);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(familyIndexHeader);
    l->addWidget(countHeader);
    l->addWidget(capabilitiesHeader);
    l->addWidget(granularityHeader);
    l->addWidget(timestampHeader);
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const Data::Queue& queue : dev.queues)
    {
        QLabel* familyIndex = new QLabel(QString::fromStdString(queue.familyIndex));
        familyIndex->setProperty("nameValueLabel", true);
        familyIndex->setMinimumWidth(FamilyIndexMaxWidth);
        familyIndex->setMaximumWidth(FamilyIndexMaxWidth);

        QLabel* count = new QLabel(QString::fromStdString(queue.queueCount));
        count->setProperty("nameValueLabel", true);
        count->setFrameShape(QFrame::Panel);
        count->setFrameShadow(QFrame::Sunken);
        count->setMinimumWidth(CountMaxWidth);
        count->setMaximumWidth(CountMaxWidth);

        QLabel* capabilities = new QLabel(QString::fromStdString(queue.capabilities));
        capabilities->setProperty("nameValueLabel", true);
        capabilities->setFrameShape(QFrame::Panel);
        capabilities->setFrameShadow(QFrame::Sunken);

        QLabel* granularity = new QLabel(QString::fromStdString(queue.minImageTransferGranularity));
        granularity->setProperty("nameValueLabel", true);
        granularity->setFrameShape(QFrame::Panel);
        granularity->setFrameShadow(QFrame::Sunken);
        granularity->setMinimumWidth(GranularityMaxWidth);
        granularity->setMaximumWidth(GranularityMaxWidth);

        QLabel* timestamp = new QLabel(QString::fromStdString(queue.timestampValidBits));
        timestamp->setProperty("nameValueLabel", true);
        timestamp->setFrameShape(QFrame::Panel);
        timestamp->setFrameShadow(QFrame::Sunken);
        timestamp->setMinimumWidth(TimestampMaxWidth);
        timestamp->setMaximumWidth(TimestampMaxWidth);

        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(familyIndex);
        l->addWidget(count);
        l->addWidget(capabilities);
        l->addWidget(granularity);
        l->addWidget(timestamp);
        layout->addLayout(l);
    }
}

/* -------------------------------------------------------------------------- *
   Update the memory UI from the data.
 * -------------------------------------------------------------------------- */
void updateMemoryUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    const int SizeMaxWidth  = 80;
    const int IndexMaxWidth = 80;
    const int FlagsMaxWidth = 200;

    QWidget* widget = ui.memory;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    QLabel* heapIndexHeader  = new QLabel("Heap Index");
    heapIndexHeader->setProperty("nameHeaderLabel", true);
    heapIndexHeader->setMinimumWidth(SizeMaxWidth);
    heapIndexHeader->setMaximumWidth(SizeMaxWidth);
    QLabel* propertiesHeader = new QLabel("Properties");
    propertiesHeader->setProperty("nameHeaderLabel", true);
    QLabel* sizeHeader  = new QLabel("Size");
    sizeHeader->setProperty("nameHeaderLabel", true);
    sizeHeader->setTextInteractionFlags(Qt::TextSelectableByMouse);
    sizeHeader->setMinimumWidth(SizeMaxWidth);
    sizeHeader->setMaximumWidth(SizeMaxWidth);
    QLabel* flagsHeader = new QLabel("Flags");
    flagsHeader->setProperty("nameHeaderLabel", true);
    flagsHeader->setMinimumWidth(FlagsMaxWidth);
    flagsHeader->setMaximumWidth(FlagsMaxWidth);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(heapIndexHeader);
    l->addWidget(sizeHeader);
    l->addWidget(propertiesHeader);
    l->addWidget(flagsHeader);
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    Data::Memory& m = dev.memory;
    for (const Data::Memory::Heap& t : m.heaps)
    {
        QLabel* heapIndex  = new QLabel(QString::fromStdString(t.index));
        heapIndex->setProperty("nameValueLabel", true);
        heapIndex->setMinimumWidth(IndexMaxWidth);
        heapIndex->setMaximumWidth(IndexMaxWidth);

        QLabel* heapProperties = new QLabel(QString::fromStdString(t.properties));
        heapProperties->setProperty("nameValueLabel", true);
        heapProperties->setFrameShape(QFrame::Panel);
        heapProperties->setFrameShadow(QFrame::Sunken);
        QLabel* size  = new QLabel(QString::fromStdString(t.size));
        size->setProperty("nameValueLabel", true);
        size->setTextInteractionFlags(Qt::TextSelectableByMouse);
        size->setMinimumWidth(SizeMaxWidth);
        size->setMaximumWidth(SizeMaxWidth);
        QLabel* flags = new QLabel(QString::fromStdString(t.flags));
        flags->setProperty("nameValueLabel", true);
        flags->setFrameShape(QFrame::Panel);
        flags->setFrameShadow(QFrame::Sunken);
        flags->setMinimumWidth(FlagsMaxWidth);
        flags->setMaximumWidth(FlagsMaxWidth);

        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(heapIndex);
        l->addWidget(size);
        l->addWidget(heapProperties);
        l->addWidget(flags);
        layout->addLayout(l);
    }
}

/* -------------------------------------------------------------------------- *
   Update the formats UI from the data.
 * -------------------------------------------------------------------------- */
void updateFormatsUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    const int FormatMaxWidth  = 80;
    const int IndexMaxWidth = 80;
    const int FlagsMaxWidth = 200;

    QWidget* widget = ui.formats;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    QLabel* formatHeader  = new QLabel("Format");
    formatHeader->setProperty("nameHeaderLabel", true);
    //formatHeader->setMinimumWidth(FormatMaxWidth);
    //formatHeader->setMaximumWidth(FormatMaxWidth);

    QLabel* linearTilingHeader = new QLabel("Linear Tiling");
    linearTilingHeader->setProperty("nameHeaderLabel", true);

    QLabel* optimalTilingHeader  = new QLabel("Optimal Tiling");
    optimalTilingHeader->setProperty("nameHeaderLabel", true);
    optimalTilingHeader->setTextInteractionFlags(Qt::TextSelectableByMouse);
    //optimalTilingHeader->setMinimumWidth(FormatMaxWidth);
    //optimalTilingHeader->setMaximumWidth(FormatMaxWidth);

    QLabel* bufferFeaturesHeader = new QLabel("Buffer Features");
    bufferFeaturesHeader->setProperty("nameHeaderLabel", true);
//    bufferFeaturesHeader->setMinimumWidth(FlagsMaxWidth);
//    bufferFeaturesHeader->setMaximumWidth(FlagsMaxWidth);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(formatHeader);
    l->addWidget(linearTilingHeader);
    l->addWidget(optimalTilingHeader);
    l->addWidget(bufferFeaturesHeader);
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const Data::Format& t : dev.formats)
    {
        QLabel* format  = new QLabel(QString::fromStdString(t.format));
        format->setProperty("nameValueLabel", true);
        //format->setMinimumWidth(IndexMaxWidth);
        //format->setMaximumWidth(IndexMaxWidth);
        format->setToolTip(QString::fromStdString(t.desc));
        format->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        QLabel* linearTiling = new QLabel(QString::fromStdString(t.linearTilingFeatures));
        linearTiling->setProperty("nameValueLabel", true);
        linearTiling->setFrameShape(QFrame::Panel);
        linearTiling->setFrameShadow(QFrame::Sunken);
        linearTiling->setAlignment(Qt::AlignTop | Qt::AlignLeft);

        QLabel* optimalTiling = new QLabel(QString::fromStdString(t.optimalTilingFeatures));
        optimalTiling->setProperty("nameValueLabel", true);
        optimalTiling->setFrameShape(QFrame::Panel);
        optimalTiling->setFrameShadow(QFrame::Sunken);
        optimalTiling->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        //optimalTiling->setMinimumWidth(FlagsMaxWidth);
        //optimalTiling->setMaximumWidth(FlagsMaxWidth);

        QLabel* bufferfeatures  = new QLabel(QString::fromStdString(t.bufferFeatures));
        bufferfeatures->setProperty("nameValueLabel", true);
        bufferfeatures->setTextInteractionFlags(Qt::TextSelectableByMouse);
        bufferfeatures->setFrameShape(QFrame::Panel);
        bufferfeatures->setFrameShadow(QFrame::Sunken);
        bufferfeatures->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        //bufferfeatures->setMinimumWidth(FormatMaxWidth);
        //bufferfeatures->setMaximumWidth(FormatMaxWidth);

        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(format);
        l->addWidget(linearTiling);
        l->addWidget(optimalTiling);
        l->addWidget(bufferfeatures);
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
    updateQueuesUi(ui, d, deviceIndex);
    updateMemoryUi(ui, d, deviceIndex);
    updateFormatsUi(ui, d, deviceIndex);

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
        ui.mainStackedWidget->setCurrentIndex(0);
        ui.capabilitiesStackedWidget->setCurrentIndex(0);

        const std::map<QToolButton*, int> buttons =
        {
            { ui.mainButton,       0 },
            { ui.extensionsButton, 1 },
            { ui.layersButton,     2 },
            { ui.featuresButton,   3 },
            { ui.memoryButton,     4 },
            { ui.queuesButton,     5 },
            { ui.limitsButton,     6 },
            { ui.formatsButton,    7 },
        };

        for (auto button : buttons)
        {
            connect(button.first, &QToolButton::clicked, [&, button]()
            { ui.capabilitiesStackedWidget->setCurrentIndex(button.second); });
        }

        connect(ui.actionAbout, &QAction::triggered, [&]()
        {
            AboutDialog dlg;
            dlg.exec();
        });
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

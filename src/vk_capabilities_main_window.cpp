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
   Creates a header label
 * -------------------------------------------------------------------------- */
QLabel* headerLabel(const QString& name, const int size = -1)
{
    QLabel* header  = new QLabel(name);
    header->setProperty("nameHeaderLabel", true); // stylesheet ID
    if (size > 0)
    {
        header->setMinimumWidth(size);
        header->setMaximumWidth(size);
    }
    return header;
}

/* -------------------------------------------------------------------------- *
   Creates a value name label
 * -------------------------------------------------------------------------- */
QLabel* valueNameLabel(const std::string& text,
                       const std::string& desc = std::string(),
                       const int size = -1)
{
    QLabel* name  = new QLabel(QString::fromStdString(text));
    name->setProperty("nameValueLabel", true);
    name->setTextInteractionFlags(Qt::TextSelectableByMouse);
    name->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    if (desc.size() > 0)
        name->setToolTip(QString::fromStdString(desc));
    if (size > 0)
    {
        name->setMinimumWidth(size);
        name->setMaximumWidth(size);
    }
    return name;
}

/* -------------------------------------------------------------------------- *
   Creates a value value label
 * -------------------------------------------------------------------------- */
QLabel* valueValueLabel(const std::string& text,
                        const std::string& desc = std::string(),
                        const int size = -1)
{
    QLabel* value = new QLabel(QString::fromStdString(text));
    value->setProperty("nameValueLabel", true);
    value->setTextInteractionFlags(Qt::TextSelectableByMouse);
    value->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    value->setFrameShape(QFrame::Panel);
    value->setFrameShadow(QFrame::Sunken);
    if (desc.size() > 0)
        value->setToolTip(QString::fromStdString(desc));
    if (size > 0)
    {
        value->setMinimumWidth(size);
        value->setMaximumWidth(size);
    }
    return value;
}

void updateEntryUi(QWidget* w, const std::vector<Data::Entry>& entries)
{
    QVBoxLayout* wLayout = new QVBoxLayout();
    delete w->layout();
    w->setLayout(wLayout);

    for (const Data::Entry& e : entries)
    {
        QHBoxLayout* hLayout = new QHBoxLayout();
        for (const Data::Cell& c : e.header.cells)
            hLayout->addWidget(headerLabel(QString::fromStdString(c.value)));
        wLayout->addLayout(hLayout);

        for (const Data::Row& r : e.valueRows)
        {
            QHBoxLayout* rLayout = new QHBoxLayout();
            for (const Data::Cell& c : r.cells)
            {
                QLabel* cellLabel = new QLabel(QString::fromStdString(c.value));
                cellLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
                cellLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
                if (c.size > 0)
                {
                    cellLabel->setMinimumWidth(c.size);
                    cellLabel->setMaximumWidth(c.size);
                }

                switch(c.style)
                {
                    case Data::Cell::Style::Header:
                        break;
                    case Data::Cell::Style::NameLabel:
                        cellLabel->setProperty("nameValueLabel", true);
                        break;
                    case Data::Cell::Style::ValueLabel:
                        cellLabel->setProperty("nameValueLabel", true);
                        cellLabel->setFrameShape(QFrame::Panel);
                        cellLabel->setFrameShadow(QFrame::Sunken);
                        break;
                    case Data::Cell::Style::ValueLabelValid:
                        cellLabel->setProperty("nameValueValidLabel", true);
                        cellLabel->setFrameShape(QFrame::Panel);
                        cellLabel->setFrameShadow(QFrame::Sunken);
                        break;
                    case Data::Cell::Style::ValueLabelInvalid:
                        cellLabel->setProperty("nameValueInvalidLabel", true);
                        cellLabel->setFrameShape(QFrame::Panel);
                        cellLabel->setFrameShadow(QFrame::Sunken);
                        break;
                }

                rLayout->addWidget(cellLabel);
            }
            wLayout->addLayout(rLayout);
        }
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

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(headerLabel("Limit"));
    l->addWidget(headerLabel("Value"));
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const Data::Limit& limit : dev.limits)
    {
        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(valueNameLabel(limit.name, limit.desc));
        l->addWidget(valueValueLabel(limit.value));
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

    const int FamilyIndexMaxWidth  =  80;
    const int CountMaxWidth        =  80;
    const int CapabilitiesMaxWidth = 270;
    const int GranularityMaxWidth  = -1;
    const int TimestampMaxWidth    = -1;

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(headerLabel("Family Index",                    FamilyIndexMaxWidth));
    l->addWidget(headerLabel("Queue count",                     CountMaxWidth));
    l->addWidget(headerLabel("Capabilities",                    CapabilitiesMaxWidth));
    l->addWidget(headerLabel("Min Image Transfer\nGranularity", GranularityMaxWidth));
    l->addWidget(headerLabel("Timestamp Valid\nBits"),          TimestampMaxWidth);
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const Data::Queue& queue : dev.queues)
    {
        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(valueValueLabel(queue.familyIndex,                 "", FamilyIndexMaxWidth));
        l->addWidget(valueValueLabel(queue.queueCount,                  "", CountMaxWidth));
        l->addWidget(valueValueLabel(queue.capabilities,                "", CapabilitiesMaxWidth));
        l->addWidget(valueValueLabel(queue.minImageTransferGranularity, "", GranularityMaxWidth));
        l->addWidget(valueValueLabel(queue.timestampValidBits,          "", TimestampMaxWidth));
        layout->addLayout(l);
    }
}

/* -------------------------------------------------------------------------- *
   Update the memory UI from the data.
 * -------------------------------------------------------------------------- */
void updateMemoryUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    const int IndexMaxWidth = 80;
    const int SizeMaxWidth  = 80;
    const int FlagsMaxWidth = 200;

    QWidget* widget = ui.memory;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(headerLabel("Heap Index", IndexMaxWidth));
    l->addWidget(headerLabel("Size", SizeMaxWidth));
    l->addWidget(headerLabel("Properties"));
    l->addWidget(headerLabel("Flags", FlagsMaxWidth));
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    Data::Memory& m = dev.memory;
    for (const Data::Memory::Heap& t : m.heaps)
    {
        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(valueValueLabel(t.index, "", IndexMaxWidth));
        l->addWidget(valueValueLabel(t.size,  "", SizeMaxWidth));
        l->addWidget(valueValueLabel(t.properties));
        l->addWidget(valueValueLabel(t.flags, "", FlagsMaxWidth));
        layout->addLayout(l);
    }
}

/* -------------------------------------------------------------------------- *
   Update the formats UI from the data.
 * -------------------------------------------------------------------------- */
void updateFormatsUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
    const int FormatMaxWidth  = 270;

    QWidget* widget = ui.formats;
    delete widget->layout();
    QVBoxLayout* layout = new QVBoxLayout();
    widget->setLayout(layout);

    QHBoxLayout* l = new QHBoxLayout();
    l->addWidget(headerLabel("Format", FormatMaxWidth));
    l->addWidget(headerLabel("Linear Tiling"));
    l->addWidget(headerLabel("Optimal Tiling"));
    l->addWidget(headerLabel("Buffer Features"));
    layout->addLayout(l);

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];
    for (const Data::Format& t : dev.formats)
    {
        QHBoxLayout* l = new QHBoxLayout();
        l->addWidget(valueNameLabel(t.format, t.desc, FormatMaxWidth));
        l->addWidget(valueValueLabel(t.linearTilingFeatures));
        l->addWidget(valueValueLabel(t.optimalTilingFeatures));
        l->addWidget(valueValueLabel(t.bufferFeatures));
        layout->addLayout(l);
    }
}

/* -------------------------------------------------------------------------- *
   Update the UI from the data.
 * -------------------------------------------------------------------------- */
void updateUi(Ui::MainWindow& ui, Data& d, int deviceIndex = -1)
{
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

    updateEntryUi(ui.properties, dev.properties);
    updateEntryUi(ui.extensions, dev.extensions);
    updateEntryUi(ui.layers,     dev.layers);
    //updatePropertiesUi(ui, d, deviceIndex);
//    updateExtensionsUi(ui, d, deviceIndex);
//    updateLayersUi(ui, d);
    //updateFeaturesUi(ui, d, deviceIndex);
    updateEntryUi(ui.features, dev.features);
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

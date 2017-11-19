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
   Returns a layout with cell label.
 * -------------------------------------------------------------------------- */
QLayout* cellLabelsLayout(const std::vector<Data::Cell>& cells)
{
    QHBoxLayout* rLayout = new QHBoxLayout();
    for (const Data::Cell& c : cells)
    {
        QLabel* cellLabel = new QLabel(QString::fromStdString(c.value));
        cellLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        cellLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        if (c.size > 0)
        {
            cellLabel->setMinimumWidth(c.size);
            cellLabel->setMaximumWidth(c.size);
        }
        if (c.desc.size())
            cellLabel->setToolTip(QString::fromStdString(c.desc));

        switch(c.style)
        {
            case Data::Cell::Style::Header:
                cellLabel->setProperty("nameHeaderLabel", true);
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
    return rLayout;
}

/* -------------------------------------------------------------------------- *
   Updates the entry UI
 * -------------------------------------------------------------------------- */
void updateEntryUi(QWidget* w, const std::vector<Data::Entry>& entries)
{
    QVBoxLayout* wLayout = new QVBoxLayout();
    delete w->layout();
    w->setLayout(wLayout);

    for (const Data::Entry& e : entries)
    {
        QLayout* hLayout = cellLabelsLayout(e.header.cells);
        wLayout->addLayout(hLayout);

        for (const Data::Row& r : e.valueRows)
        {
            QLayout* rLayout = cellLabelsLayout(r.cells);
            wLayout->addLayout(rLayout);
        }
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
    updateEntryUi(ui.features,   dev.features);
    updateEntryUi(ui.queues,     dev.queues);
    updateEntryUi(ui.memory,     dev.memories);
    updateEntryUi(ui.formats,    dev.formats);
    updateEntryUi(ui.limits,     dev.limits);

    QMenu* deviceMenu = new QMenu();
    deviceMenu->setFont(QFont("Segoe UI", 10));
    for (const Data::PhysicalDeviceData& dev : d.physicalDeviceData)
        deviceMenu->addAction(QString::fromStdString(dev.name));

    ui.deviceButton->blockSignals(true);
    ui.deviceButton->setMenu(deviceMenu);
    ui.deviceButton->setText(QString::fromStdString(dev.name));
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

/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::MainWindow class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_main_window.h"
#include "ui_vk_capabilities_main_window.h"

/* -------------------------------------------------------------------------- */

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>

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
   Returns true if the row is a multiline row.
 * -------------------------------------------------------------------------- */
bool isMultiline(const std::vector<Data::Cell>& cells)
{
    bool out = false;
    for (const Data::Cell& c : cells)
    {
        QString value = QString::fromStdString(c.value);
        if (value.contains("\n"))
        {
            out = true;
            break;
        }
    }
    return out;
}

/* -------------------------------------------------------------------------- *
   Update table widgets from entry data.
 * -------------------------------------------------------------------------- */
void updateEntryUi(std::vector<QTableWidget*> tableWidgets,
                   const std::vector<Data::Entry>& entries)
{
    if (tableWidgets.size() != entries.size())
        return;

    for (int i = 0; i < entries.size(); ++i)
    {
        const Data::Entry& e = entries[i];
        if (e.valueRows.size() == 0)
            continue;

        QTableWidget* w = tableWidgets[i];
        w->setRowCount(int(e.valueRows.size()));
        w->setColumnCount(int(e.valueRows.front().cells.size()));
        w->setWordWrap(true);
        w->setTextElideMode(Qt::ElideMiddle);
        w->verticalHeader()->setVisible(false);

        w->horizontalHeader()->setVisible(true);

        for (int rIndex = 0; rIndex < e.valueRows.size(); ++rIndex)
        {
            const Data::Row& r = e.valueRows[rIndex];
            for (int cIndex = 0; cIndex < r.cells.size(); ++cIndex)
            {
                const Data::Cell& c = r.cells[cIndex];

                QTableWidgetItem* item = new QTableWidgetItem();
                item->setText(QString::fromStdString(c.value));

                if (c.desc.size())
                {
                    QString tooltip = "<font size='4'>";
                    tooltip.append(QString::fromStdString(c.desc));
                    tooltip.append("</font>");
                    item->setToolTip(tooltip);
                }

                if (c.style == Data::Cell::Style::ValueLabelValid)
                {
                    item->setFont(QFont("Segoe UI", 10, QFont::Bold));
                }
                else if (c.style == Data::Cell::Style::ValueLabelInvalid)
                {
                    item->setFont(QFont("Segoe UI", 10));
                }
                w->setItem(rIndex, cIndex, item);
            }
        }

        w->resizeColumnsToContents();
        w->resizeRowsToContents();
        for (int c = 0; c < w->horizontalHeader()->count(); ++c)
            w->horizontalHeader()->setSectionResizeMode(
                c, QHeaderView::Interactive);
    }
}

/* -------------------------------------------------------------------------- *
   Update the UI from the data.
 * -------------------------------------------------------------------------- */
void updateUi(QMainWindow* mainWindow,
              Ui::MainWindow& ui,
              Data& d,
              bool uiThread,
              int deviceIndex = -1)
{ 
    Qt::ConnectionType connectionType =
        uiThread ? Qt::AutoConnection
                 : Qt::BlockingQueuedConnection;

    if (!d.hasVulkan)
    {
        QMetaObject::invokeMethod(
            mainWindow,
            "doSetNoVulkan",
            connectionType);

        return;
    }

    if (!d.physicalDeviceData.size())
    {
        QMetaObject::invokeMethod(
            mainWindow,
            "doSetNoPhysicalDevices",
            connectionType);

        return;
    }

    if (deviceIndex == -1)
        deviceIndex = 0;

    Data::PhysicalDeviceData& dev = d.physicalDeviceData[deviceIndex];   

    std::vector<QTableWidget*> propertiesTableWidgets =
    { ui.propertiesTableWidget };
    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, propertiesTableWidgets),
        Q_ARG(std::vector<Data::Entry>, dev.properties));

    std::vector<QTableWidget*> instanceWidgets =
    {
        ui.instanceExtensionsTableWidget,
        ui.deviceExtensionsTableWidget
    };
    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, instanceWidgets ),
        Q_ARG(std::vector<Data::Entry>, dev.extensions));

    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, { ui.layersTableWidget } ),
        Q_ARG(std::vector<Data::Entry>, dev.layers));

    std::vector<QTableWidget*> featuresTableWidgets =
    { ui.featuresTableWidget };
    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, featuresTableWidgets),
        Q_ARG(std::vector<Data::Entry>, dev.features));

    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, { ui.queuesTableWidget } ),
        Q_ARG(std::vector<Data::Entry>, dev.queues));

    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
                Q_ARG(std::vector<QTableWidget*>, { ui.memoryTableWidget } ),
                Q_ARG(std::vector<Data::Entry>, dev.memories));

    std::vector<QTableWidget*> formatTableWidgets =
    { ui.formatsTableWidget, ui.formatsTableWidget2, ui.formatsTableWidget3 };

    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, formatTableWidgets),
        Q_ARG(std::vector<Data::Entry>, dev.formats));

    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, { ui.limitsTableWidget } ),
        Q_ARG(std::vector<Data::Entry>, dev.limits));

    std::vector<QTableWidget*> surfaceTableWidgets =
    { ui.surfaceTableWidget, ui.surfaceFormatsTableWidget };

    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdateEntryUi",
        connectionType,
        Q_ARG(std::vector<QTableWidget*>, surfaceTableWidgets ),
        Q_ARG(std::vector<Data::Entry>, dev.surface));

    QStringList devices;
    for (Data::PhysicalDeviceData& dev : d.physicalDeviceData)
        devices.push_back(QString::fromStdString(dev.name));

    QMetaObject::invokeMethod(
        mainWindow,
        "doUpdatePhysicalDeviceButtonMenu",
        connectionType,
        Q_ARG(QStringList, devices));
}

} // anonymous namespace

/* -------------------------------------------------------------------------- */

struct MainWindow::Impl
{
    Impl(MainWindow* self)
    {
        qRegisterMetaType<std::vector<Data::Entry>>("std::vector<Data::Entry>");

        ui.setupUi(self);
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
            { ui.surfaceButton,    8 },
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

        connect(ui.actionDeviceTest, &QAction::triggered,
                self, &MainWindow::doRunDevicetest);
    }

    // UI controls
    Ui::MainWindow ui;
    // Progress dialog.
    QProgressDialog progress;
    // Data model.
    std::shared_ptr<Data> data;
    // Device index.
    int deviceIndex = 0;
};

/* -------------------------------------------------------------------------- */

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , impl(std::make_shared<Impl>(this))
{}

/* -------------------------------------------------------------------------- */

void MainWindow::showProgress()
{
    QProgressDialog& progress = impl->progress;
    Qt::WindowFlags flags = progress.windowFlags();
    flags = flags & (~Qt::WindowContextHelpButtonHint); // '?' icon
    flags = flags & (~Qt::WindowCloseButtonHint);       // 'X' icon
    progress.setWindowFlags(flags);
    progress.setRange(0, 9);
    progress.setValue(0);
    progress.setCancelButton(nullptr);
    progress.setWindowTitle("Vulkan capabilities...");
    progress.setLabelText("Checking Vulkan capabilities... please wait...");
    progress.show();

    auto progressUpdate = [&]()
    { progress.setValue(progress.value() + 1); };

    QObject::connect(this,
                     &MainWindow::updateProgress,
                     progressUpdate);

    QApplication::processEvents(
        QEventLoop::ExcludeUserInputEvents);
}

/* -------------------------------------------------------------------------- */

void MainWindow::hideProgress()
{
    QProgressDialog& progress = impl->progress;
    progress.hide();
    progress.reset();

    QApplication::processEvents(
        QEventLoop::ExcludeUserInputEvents);
}

/* -------------------------------------------------------------------------- */

void MainWindow::setDataAsync(std::shared_ptr<Data> data)
{
    impl->data = data;
    updateUi(this, impl->ui, *impl->data, false);
}

/* -------------------------------------------------------------------------- */

void MainWindow::setData(std::shared_ptr<Data> data)
{
    showProgress();
    impl->data = data;
    updateUi(this, impl->ui, *impl->data, true, impl->deviceIndex);
    QApplication::processEvents(
        QEventLoop::ExcludeUserInputEvents);
    hideProgress();
}

/* -------------------------------------------------------------------------- */

void MainWindow::doRunDevicetest()
{
    emit runDeviceTest(impl->deviceIndex);
}

/* -------------------------------------------------------------------------- */

void MainWindow::doSetNoVulkan()
{
    QMessageBox::critical(
        this, "No Vulkan Implementation",
        "Vulkan implementation is missing.");
}

/* -------------------------------------------------------------------------- */

void MainWindow::doSetNoPhysicalDevices()
{
    QMessageBox::critical(
        this, "No physical devices",
        "No physical devices with Vulkan capabilities.");
}

/* -------------------------------------------------------------------------- */

void MainWindow::doUpdatePhysicalDeviceButtonMenu(const QStringList& devices)
{
    QMenu* deviceMenu = new QMenu();
    deviceMenu->setFont(QFont("Segoe UI", 10));
    for (int deviceIndex = 0; deviceIndex < devices.size(); ++deviceIndex)
    {
        const QString dev = devices[deviceIndex];
        QAction* a = deviceMenu->addAction(dev);
        a->setProperty("device_index", deviceIndex);

        connect(a, &QAction::triggered, [&, deviceIndex]()
        {
            QMetaObject::invokeMethod(
                this,
                "doSelectPhysicalDevice",
                Q_ARG(int, deviceIndex));
        });
    }

    QString devText = "No Device";
    if (impl->deviceIndex >= 0 && impl->deviceIndex < devices.size())
        devText = devices[impl->deviceIndex];

    impl->ui.deviceButton->blockSignals(true);
    impl->ui.deviceButton->setMenu(deviceMenu);
    impl->ui.deviceButton->setText(devText);
    impl->ui.deviceButton->blockSignals(false);
}

/* -------------------------------------------------------------------------- */

void MainWindow::doUpdateEntryUi(
    std::vector<QTableWidget*> widgets,
    const std::vector<Data::Entry>& entry)
{
    emit updateProgress();
    QApplication::processEvents();
    updateEntryUi(widgets, entry);
    QApplication::processEvents();
}

/* -------------------------------------------------------------------------- */

void MainWindow::doSelectPhysicalDevice(int index)
{
    impl->deviceIndex = index;
    setData(impl->data);
}

} // namespace vk_capabilities
} // namespace kuu

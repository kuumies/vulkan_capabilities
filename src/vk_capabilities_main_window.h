/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_capabilities::MainWindow class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <QtWidgets/QMainWindow>

/* -------------------------------------------------------------------------- */

#include "vk_capabilities_data.h"

/* -------------------------------------------------------------------------- */

class QTableWidget;

namespace kuu
{
namespace vk_capabilities
{

/* -------------------------------------------------------------------------- */

struct Data;

/* -------------------------------------------------------------------------- *
   The main window of Vulkan Capabilities application.
 * -------------------------------------------------------------------------- */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructs the main window with an optional parent widget.
    explicit MainWindow(QWidget* parent = nullptr);

    // Show the progress
    void showProgress();
    // Hides the progress
    void hideProgress();

    // Sets the data model to fill the UI fields. If the system does not
    // contain a Vulkan implementation then a special message about this
    // is visible. It is assumed than the data is set from non-UI thread.
    void setDataAsync(std::shared_ptr<Data> data);
    void setData(std::shared_ptr<Data> data);

signals:
    void updateProgress();
    void runDeviceTest(int deviceIndex);

private slots:
    void doRunDevicetest();
    void doSetNoVulkan();
    void doSetNoPhysicalDevices();
    void doUpdatePhysicalDeviceButtonMenu(const QStringList &devices);
    void doUpdateEntryUi(std::vector<QTableWidget*> widgets,
                         const std::vector<Data::Entry>& entry);
    void doSelectPhysicalDevice(int index);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk_capabilities
} // namespace kuu


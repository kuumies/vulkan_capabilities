/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_capabilities::MainWindow class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <QtWidgets/QMainWindow>

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

    // Sets the data model to fill the UI fields. If the system does not
    // contain a Vulkan implementation then a special message about this
    // is visible.
    void setData(std::shared_ptr<Data> data);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk_capabilities
} // namespace kuu

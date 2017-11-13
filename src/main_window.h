/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk::MainWindow class
 * ---------------------------------------------------------------- */

#pragma once

#include <memory>
#include <QtWidgets/QMainWindow>

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- *
   The main window of Vulkan test application.
 * ---------------------------------------------------------------- */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructs the main window with an optional parent widget.
    explicit MainWindow(QWidget* parent = nullptr);

private:
    struct Data;
    std::shared_ptr<Data> d;
};

} // namespace vk
} // namespace kuu

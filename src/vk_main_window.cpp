/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::MainWindow class
 * ---------------------------------------------------------------- */

#include "vk_main_window.h"
#include "ui_vk_main_window.h"

namespace kuu
{
namespace vk
{

struct MainWindow::Data
{
    Data(MainWindow* self)
    {
        ui.setupUi(self);
    }

    Ui::MainWindow ui;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d(std::make_shared<Data>(this))
{}

} // namespace vk
} // namespace kuu

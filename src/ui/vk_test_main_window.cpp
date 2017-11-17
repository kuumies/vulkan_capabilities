/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_test::MainWindow class
 * -------------------------------------------------------------------------- */

#include "vk_test_main_window.h"
#include "ui_vk_test_main_window.h"

/* -------------------------------------------------------------------------- */

#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

namespace kuu
{
namespace vk_test
{

/* -------------------------------------------------------------------------- */

struct MainWindow::Data
{
    Data(MainWindow* self)
    {
        ui.setupUi(self);
    }

    Ui::MainWindow ui;
};

/* -------------------------------------------------------------------------- */

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , d(std::make_shared<Data>(this))
{}

} // namespace vk_test
} // namespace kuu

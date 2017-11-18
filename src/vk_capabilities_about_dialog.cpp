/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk_capabilities::AboutDialog class
 * -------------------------------------------------------------------------- */

#include "vk_capabilities_about_dialog.h"
#include "ui_vk_capabilities_about_dialog.h"

namespace kuu
{
namespace vk_capabilities
{

struct AboutDialog::Impl
{
    Ui::AboutDialog ui;
};

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
    , impl(std::make_shared<Impl>())
{
    impl->ui.setupUi(this);
}

} // namespace vk_capabilities
} // namespace kuu

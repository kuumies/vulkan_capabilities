/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk_capabilities::AboutDialog class
 * -------------------------------------------------------------------------- */

#pragma once

/* -------------------------------------------------------------------------- */

#include <memory>
#include <QtWidgets/QDialog>

namespace kuu
{
namespace vk_capabilities
{

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

private:
    struct Impl;
    std::shared_ptr<Impl> impl;
};

} // namespace vk_capabilities
} // namespace kuu

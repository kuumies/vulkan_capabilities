/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Vulkan Capabilities application.
 * ---------------------------------------------------------------- */

#include <vulkan/vulkan.h>
#include <QtWidgets/QApplication>

/* ---------------------------------------------------------------- */

#include "vk_capabilities_controller.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    kuu::vk_capabilities::Controller controller;
    controller.showUi();

    return app.exec();
}

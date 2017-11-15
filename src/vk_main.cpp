/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Vulkan test application.
 * ---------------------------------------------------------------- */

#include <vulkan/vulkan.h>
#include <QtWidgets/QApplication>
#include "vk_test_controller.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    kuu::vk_test::Controller controller;
    controller.runVulkanTest();

    return app.exec();
}

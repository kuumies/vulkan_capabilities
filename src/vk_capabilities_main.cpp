/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Vulkan Capabilities application.
 * ---------------------------------------------------------------- */

#include <iostream>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <vulkan/vulkan.h>

/* ---------------------------------------------------------------- */

#include "vk_capabilities_controller.h"

/* ---------------------------------------------------------------- */

QString readStyleSheet()
{
    QFile qssFile("://stylesheets/stylesheet.qss");
    if (!qssFile.open(QIODevice::ReadOnly))
    {
        std::cerr << __FUNCTION__
                  << "Failed to read stylesheet from resource"
                  << std::endl;
        return QString();
    }

    return QTextStream(&qssFile).readAll();
}

/* ---------------------------------------------------------------- */

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setStyleSheet(readStyleSheet());
    app.setWindowIcon(QIcon("://icons/kuu.png"));

    kuu::vk_capabilities::Controller controller;
    controller.showUi();

    return app.exec();
}

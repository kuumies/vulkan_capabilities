#pragma once

#include <QtWidgets/QWidget>
#include <QWindow>
#include <vulkan/vulkan.h>

class Widget : public QWidget
{
public:
    Widget(QWidget* parent = nullptr);

    bool eventFilter(QObject *, QEvent *) override;
    void paintEvent(QPaintEvent* e);

    void setAsVulkanSurface(VkInstance instance);

    VkSurfaceKHR surface;
};

#pragma once

#include <QtWidgets/QWidget>
#include <QWindow>
#include <vulkan/vulkan.h>

class Widget : public QWindow
{
public:
    Widget(QWidget* parent = nullptr);
    void setAsVulkanSurface(VkInstance instance);

    VkSurfaceKHR surface;
};

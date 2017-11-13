#include "widget.h"
#include <vulkan/vulkan.h>
#include <QWindow>
#include <windows.h>
#include <qpa/qplatformnativeinterface.h>
#include <QGuiApplication>
#include <iostream>

typedef VkFlags VkWin32SurfaceCreateFlagsKHR;
typedef struct VkWin32SurfaceCreateInfoKHR
{
    VkStructureType                 sType;
    const void*                     pNext;
    VkWin32SurfaceCreateFlagsKHR    flags;
    HINSTANCE                       hinstance;
    HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;

typedef VkResult (APIENTRY *PFN_vkCreateWin32SurfaceKHR)(VkInstance,const VkWin32SurfaceCreateInfoKHR*,const VkAllocationCallbacks*,VkSurfaceKHR*);

Widget::Widget(QWidget* parent)
    //: QWidget(parent)
{
    resize(720, 576);
}

void Widget::setAsVulkanSurface(VkInstance instance)
{
    //HWND handle = getHWNDForWidget(this);
    HWND handle = (HWND) winId();
    std::cout << handle << std::endl;

    VkWin32SurfaceCreateInfoKHR createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = handle;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR) vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");

    if (!CreateWin32SurfaceKHR || CreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS)
    {
        std::cerr << "failed to create window surface!" << std::endl;
    }
}

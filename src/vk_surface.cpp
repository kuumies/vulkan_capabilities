/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Surface class
 * ---------------------------------------------------------------- */

#include "vk_surface.h"

/* ---------------------------------------------------------------- */

#include <glfw/glfw3.h>

/* ---------------------------------------------------------------- */

#include "vk_instance.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct Surface::Data
{
    Instance instance;
    GLFWwindow* window;
    VkSurfaceKHR surface;
};

/* ---------------------------------------------------------------- */

Surface::Surface(const Instance& instance)
    : d(std::make_shared<Data>())
{
    d->instance = instance;
}

/* ---------------------------------------------------------------- */

bool Surface::create(GLFWwindow* window)
{
    d->window = window;
    const VkResult result = glfwCreateWindowSurface(
        d->instance.handle(),
        d->window,
        nullptr,
        &d->surface);

    return result == VK_SUCCESS;
}

/* ---------------------------------------------------------------- */

bool Surface::destroy()
{
    vkDestroySurfaceKHR(d->instance.handle(), d->surface, nullptr);
    return true;
}

/* ---------------------------------------------------------------- */

bool Surface::areExtensionsSupported()
{
    for (const std::string& ex : extensions())
        if (!Instance::isExtensionSupported(ex))
            return false;
    return true;
}

/* ---------------------------------------------------------------- */

std::vector<std::string> Surface::extensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions =
        glfwGetRequiredInstanceExtensions(
            &glfwExtensionCount);

    std::vector<std::string> out;
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
        out.push_back(std::string(glfwExtensions[i]));
    return out;
}

} // namespace vk
} // namespace kuu

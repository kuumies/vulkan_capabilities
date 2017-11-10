/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Surface class
 * ---------------------------------------------------------------- */

#include "vk_surface.h"

/* ---------------------------------------------------------------- */

#include <iostream>
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
    Data(const Instance& instance, GLFWwindow* window)
        : instance(instance)
        , valid(false)
    {
        const VkResult result = glfwCreateWindowSurface(
            instance.handle(),
            window,
            nullptr,
            &surface);

        if (result == VK_SUCCESS)
        {
            valid = true;
        }
        else
        {
            std::cerr << __FUNCTION__
                      << ": failed to create surface for a window"
                      << std::endl;
            valid = false;
        }
    }

     ~Data()
    {
        vkDestroySurfaceKHR(instance.handle(), surface, nullptr);
    }

    const Instance instance;
    VkSurfaceKHR surface;
    bool valid = false;
};

/* ---------------------------------------------------------------- */

Surface::Surface(const Instance& instance, GLFWwindow* window)
    : d(std::make_shared<Data>(instance, window))
{}

/* ---------------------------------------------------------------- */

bool Surface::isValid() const
{ return d->valid; }

/* ---------------------------------------------------------------- */

VkSurfaceKHR Surface::handle() const
{ return d->surface; }

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

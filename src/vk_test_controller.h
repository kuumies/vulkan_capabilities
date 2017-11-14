/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The definition of kuu::vk:_test::Controller class
 * -------------------------------------------------------------------------- */
 
#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace kuu
{
namespace vk_test
{

/* ---------------------------------------------------------------- *
   A controller for Vulkan Test application.
 * ---------------------------------------------------------------- */
class Controller
{
public:
    // Constructs the controller.
    Controller();
    // Destroys the controller.
    ~Controller();

private:
    void createInstance();
    void destroyInstance();
    
    void enumeratePhysicalDevices();
    
private:
    VkInstance instance = VK_NULL_HANDLE;
};

} // namespace vk_test
} // namespace kuu

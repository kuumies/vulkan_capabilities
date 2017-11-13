/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The main entry point of Vulkan test application.
 * ---------------------------------------------------------------- */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>
#include <memory>
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vulkan/vulkan.h>
#include <QtWidgets/QApplication>

/* ---------------------------------------------------------------- */

#include "vk_command_buffer.h"
#include "vk_device.h"
#include "vk_instance.h"
#include "vk_mesh.h"
#include "vk_physical_device.h"
#include "vk_pipeline.h"
#include "vk_queue.h"
#include "vk_render_pass.h"
#include "vk_semaphore.h"
#include "vk_shader_stage.h"
#include "vk_surface.h"
#include "vk_swap_chain.h"
#include "widget.h"

/* ---------------------------------------------------------------- *
   Globals.
 * ---------------------------------------------------------------- */

// Window
static const char* WINDOW_NAME = "Vulkan test";
static const int WINDOW_WIDTH  = 720;
static const int WINDOW_HEIGHT = 576;

// Surface
static const VkFormat SURFACE_FORMAT             = VK_FORMAT_B8G8R8A8_UNORM;
static const VkColorSpaceKHR SURFACE_COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// Presentation
static const VkPresentModeKHR PRESENT_MODE = VK_PRESENT_MODE_FIFO_KHR; // v-sync

// Swap chain
static const int SWAP_CHAIN_IMAGE_COUNT = 2;

int main(int argc, char* argv[])
{
    // GLFW window.
//    glfwInit();
//    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//    glfwWindowHint(GLFW_RESIZABLE,  GLFW_FALSE);
//    GLFWwindow* window = glfwCreateWindow(
//        WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME,
//        nullptr, nullptr);
//    if (!window)
//    {
//        std::cerr << __FUNCTION__
//                  << ": Failed to create Vulkan GLFW window"
//                  << std::endl;

//        return EXIT_FAILURE;
//    }

//    uint32_t glfwExtensionCount = 0;
//    const char** glfwExtensions =
//        glfwGetRequiredInstanceExtensions(
//            &glfwExtensionCount);

//    std::vector<std::string> out;
//    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
//        out.push_back(std::string(glfwExtensions[i]));
//    for (auto s : out)
//        std::cout << s << std::endl;

    using namespace kuu::vk;

    // Create an instance parameters.
    Instance::Parameters params;
    params.applicationName = WINDOW_NAME;
    params.engineName = "kuuEngine";
    params.createValidationLayer = true;
    params.createSurfaceExtensesions = true;

    // Create an instance.
    Instance instance(params);
    if (!instance.isValid())
        return EXIT_FAILURE;


    QApplication app(argc, argv);
    Widget w;
    w.setAsVulkanSurface(instance.handle());
    w.show();

    // Create a surface.
    //Surface surface = instance.createSurface(window);
    Surface surface(instance, w.surface);
    if (!surface.isValid())
        return EXIT_FAILURE;
    surface.setFormat( { SURFACE_FORMAT, SURFACE_COLOR_SPACE });
    surface.setImageExtent( { WINDOW_WIDTH, WINDOW_HEIGHT } );
    surface.setSwapChainImageCount(SWAP_CHAIN_IMAGE_COUNT);
    surface.setPresentMode(PRESENT_MODE);

    // Get all physical devices.
    std::vector<PhysicalDevice> physicalDevices =
        instance.physicalDevices();

    // Find physical devices that supports the needed properties.
    std::vector<std::string> extensionNames;
    extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    std::vector<PhysicalDevice> suitablePhysicalDevices;
    for (const PhysicalDevice& device : physicalDevices)
    {
        if (!device.isExtensionSupported(extensionNames))
            continue;

        if (!surface.isCompatibleWith(device))
            surface.setCompatibleWith(device);

        suitablePhysicalDevices.push_back(device);
        break;
    }

    if (suitablePhysicalDevices.size() == 0)
    {
        std::cerr << __FUNCTION__
                  << ": failed to find suitable physical device "
                  << std::endl;

        return EXIT_FAILURE;
    }

    // Auto-select the first device.
    PhysicalDevice physicalDevice = suitablePhysicalDevices[0];

    // Create logical device queue parameteres.
    std::vector<Device::QueueParameters> queueParameters =
    {
        { Queue::Type::Graphics,     1, 1.0f, nullptr  },
        { Queue::Type::Presentation, 1, 1.0f, &surface }
    };

    // Create logical device.
    Device device(physicalDevice, queueParameters, extensionNames);
    if (device.isValid())
        return EXIT_FAILURE;

    // Create swap chain.
    SwapChain swapChain = device.createSwapChain(surface);
    if (!swapChain.isValid())
        return EXIT_FAILURE;

    //Create shaders
    ShaderStage shader(device.handle());
    shader.create("shaders/test.vert.spv",
                  "shaders/test.frag.spv");

    // Create mesh
    const std::vector<Vertex> vertices =
    {
        {  {0.0f, -0.5f}, { 1.0f, 0.0f, 0.0f } },
        {  {0.5f,  0.5f}, { 0.0f, 1.0f, 0.0f } },
        { {-0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f } }
    };
    Mesh mesh(device.handle(), physicalDevice.handle(), 0);
    mesh.create(vertices);

    // Create pipeline
    Pipeline::Parameters pipelineParams(mesh, shader, swapChain);
    pipelineParams.viewportWidth  = WINDOW_WIDTH;
    pipelineParams.viewportHeight = WINDOW_HEIGHT;
    Pipeline pipeline(device, surface, pipelineParams);

    // Get render pass.
    RenderPass renderPass = pipeline.renderPass();
    if (!renderPass.isValid())
        return EXIT_FAILURE;

    // Get the graphics queue.
    Queue graphicsQueue = device.queue(Queue::Type::Graphics);

    // Create command buffer.
    CommandBuffer commandBuffer(device, graphicsQueue,
                                swapChain.imageViewSize());
    if (!commandBuffer.isValid())
        return EXIT_FAILURE;

    // Same commands are used to for each framebufferr
    for (int i = 0; i < commandBuffer.bufferCount(); i++)
    {
        VkCommandBuffer buffer = commandBuffer.buffer(i);
        commandBuffer.begin(i);

        VkClearValue clearColor = { 0.2f, 0.2f, 0.2f, 1.0f };
        renderPass.begin(i, buffer, clearColor);

        pipeline.bind(buffer);
        mesh.draw(buffer);

        renderPass.end(buffer);

        commandBuffer.end(i);
    }

    // Create sync
    Semaphore imageAvailableSemaphore(device);
    if (!imageAvailableSemaphore.isValid())
        return EXIT_FAILURE;
    Semaphore renderFinishedSemaphore(device);
    if (!renderFinishedSemaphore.isValid())
        return EXIT_FAILURE;

    // Acquire an image from the swap chain. Use the semaphore to
    // detect when the present queue has finished using the image.
    // The image is a color attachment to framebuffer.
    uint32_t imageIndex = swapChain.acquireImage(imageAvailableSemaphore);

    // Submit the command buffer into queue
    graphicsQueue.submit(commandBuffer.buffer(imageIndex),
                         imageAvailableSemaphore,
                         renderFinishedSemaphore);

    // Get the present queue from the logical device.
    Queue presentQueue = device.queue(Queue::Type::Presentation);
    // Present the framebuffer from swap chain into screen surface.
    presentQueue.present(swapChain,
                         imageIndex,
                         renderFinishedSemaphore);
    presentQueue.waitIdle();


    /* ------------------------------------------------------------ *
       Start the "event" loop.
     * ------------------------------------------------------------ */

//    while(!glfwWindowShouldClose(window))
//    {
//        glfwPollEvents();
//    }

    app.exec();

    vkDeviceWaitIdle(device.handle());

    /* ------------------------------------------------------------ *
       Clean up
     * ------------------------------------------------------------ */

    //glfwDestroyWindow(window);
    //glfwTerminate();

    shader.destroy();
    mesh.destroy();

    return EXIT_SUCCESS;
}

/* ---------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::SwapChain class
 * ---------------------------------------------------------------- */

#include "vk_swap_chain.h"

/* ---------------------------------------------------------------- */

#include <iostream>

/* ---------------------------------------------------------------- */

#include "vk_surface.h"
#include "vk_device.h"

namespace kuu
{
namespace vk
{

/* ---------------------------------------------------------------- */

struct SwapChain::Data
{
    Data(const Device& device, const Surface& surface)
        : device(device)
    {
        Queue graphicsQueue = device.queue(Queue::Type::Graphics);
        Queue presentQueue  = device.queue(Queue::Type::Presentation);

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = surface.handle();
        createInfo.minImageCount    = surface.swapChainImageCount();
        createInfo.imageFormat      = surface.format().format;
        createInfo.imageColorSpace  = surface.format().colorSpace;
        createInfo.imageExtent      = surface.imageExtent();
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode      = surface.presentMode();
        createInfo.clipped          = VK_TRUE;
        createInfo.oldSwapchain     = VK_NULL_HANDLE;
        createInfo.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

        std::vector<uint32_t> queueFamilyIndices;
        queueFamilyIndices.push_back(graphicsQueue.familyIndex());
        queueFamilyIndices.push_back(presentQueue.familyIndex());

        if (graphicsQueue.familyIndex() != presentQueue.familyIndex())
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = uint32_t(queueFamilyIndices.size());
            createInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkResult result = vkCreateSwapchainKHR(
            device.handle(),
            &createInfo,
            nullptr,
            &swapChain);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": failed to create vulkan swap chain"
                      << std::endl;
            return;
        }

        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(
            device.handle(),
            swapChain,
            &imageCount,
            nullptr);

        std::vector<VkImage> swapChainImages(imageCount);
        vkGetSwapchainImagesKHR(
            device.handle(),
            swapChain,
            &imageCount,
            swapChainImages.data());

        imageViews.resize(imageCount);
        for (uint32_t i = 0; i < imageCount; ++i)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image    = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format   = surface.format().format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(device.handle(),
                                       &createInfo,
                                       nullptr,
                                       &imageViews[i]);
            if (result != VK_SUCCESS)
            {
                std::cerr << __FUNCTION__
                          << ": failed to create vulkan image view"
                          << std::endl;

                return;
            }
        }

        valid = true;
    }

    ~Data()
    {
        for (uint32_t i = 0; i < imageViews.size(); i++)
            vkDestroyImageView(device.handle(), imageViews[i], nullptr);
        vkDestroySwapchainKHR(device.handle(), swapChain, nullptr);
    }

    Device device;
    VkSwapchainKHR swapChain;
    std::vector<VkImageView> imageViews;

    bool valid = false;
};

/* ---------------------------------------------------------------- */

SwapChain::SwapChain(const Device& device, const Surface& surface)
    : d(std::make_shared<Data>(device, surface))
{}

/* ---------------------------------------------------------------- */

bool SwapChain::isValid() const
{ return d->valid; }

/* ---------------------------------------------------------------- */

VkSwapchainKHR SwapChain::handle() const
{ return d->swapChain; }

/* ---------------------------------------------------------------- */

std::vector<VkImageView> SwapChain::imageViews() const
{ return d->imageViews; }

} // namespace vk
} // namespace kuu

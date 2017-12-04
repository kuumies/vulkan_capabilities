/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Swapchain class
 * -------------------------------------------------------------------------- */

#include "vk_swapchain.h"
#include <algorithm>
#include <iostream>
#include "vk_image.h"
#include "vk_stringify.h"

/* -------------------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

struct Swapchain::Impl
{
    Impl(const VkDevice& logicalDevice,
         const VkSurfaceKHR& surface)
        : surface(surface)
        , logicalDevice(logicalDevice)
    {}

    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        auto queueIndicesUnique = queueIndices;
        std::sort(queueIndicesUnique.begin(), queueIndicesUnique.end());
        auto it = std::unique(queueIndicesUnique.begin(), queueIndicesUnique.end());
        queueIndicesUnique.erase(it, queueIndicesUnique.end());

        bool indicesDiffer = queueIndicesUnique.size() > 1;
        const VkSharingMode imageSharingMode =
            indicesDiffer ? VK_SHARING_MODE_CONCURRENT
                          : VK_SHARING_MODE_EXCLUSIVE;

        // Fill create info.
        VkStructureType type = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        VkSwapchainCreateInfoKHR info;
        info.sType                 = type;                                // Must be this type
        info.pNext                 = NULL;                                // No extension features
        info.flags                 = 0;                                   // Must be 0
        info.surface               = surface;                             // Surface handle
        info.imageArrayLayers      = 1;                                   // Non-stereoscopic
        info.clipped               = VK_TRUE;                             // Allow window system to clip the framebuffer content
        info.oldSwapchain          = VK_NULL_HANDLE;                      // No old swap chain.
        info.minImageCount         = imageCount;                          // Image count
        info.imageFormat           = surfaceFormat.format;                // Image format
        info.imageColorSpace       = surfaceFormat.colorSpace;            // Image color space
        info.imageExtent           = imageExtent;                         // Image extent
        info.imageArrayLayers      = 1;                                   // No stereoscope
        info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Swap chain images are used as color attachments.
        info.imageSharingMode      = imageSharingMode;                    // Image sharing mode
        info.queueFamilyIndexCount = uint32_t(queueIndices.size());       // Queue family index count
        info.pQueueFamilyIndices   = queueIndices.data();                 // Queue family indices
        info.preTransform          = preTransform;                        // Pre-transform
        info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;   // No composition
        info.presentMode           = presentMode;                         // Present mode
        info.clipped               = VK_TRUE;                             // Allow other widget to clip the surface
        info.oldSwapchain          = VK_NULL_HANDLE;                      // No old swap chain

        // Create swapchain
        const VkResult result =
            vkCreateSwapchainKHR(
                logicalDevice, // [in]  logical device.
                &info,         // [in]  info
                NULL,          // [in]  allocator
                &swapchain);   // [out] swapchain handle
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": swap chain creation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        // Get swapchain image count
        uint32_t swapchainImageCount;
        vkGetSwapchainImagesKHR(
            logicalDevice,        // [in]  Logical device handle
            swapchain,            // [in]  Swapchain handle
            &swapchainImageCount, // [out] Swapchain image count
            NULL);                // [in]  Allocator

        swapchainImages.resize(swapchainImageCount);

        // Get swapchain images
        vkGetSwapchainImagesKHR(
            logicalDevice,           // [in]  Logical device handle
            swapchain,               // [in]  Swapchain handle
            &swapchainImageCount,    // [out] Swapchain image count
            swapchainImages.data()); // [in]  Allocator

        // Create swapchain image views
        swapchainImageViews.resize(swapchainImageCount);
        for (size_t i = 0; i < swapchainImageCount; ++i)
        {
            // Color image, no mipmapping, no layers
            VkImageSubresourceRange subresourceRange =
            {  VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

            // No remapping
            VkComponentMapping components =
            { VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY,
              VK_COMPONENT_SWIZZLE_IDENTITY };

            VkStructureType type = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            VkImageViewCreateInfo info;
            info.sType            = type;                  // Must be this type
            info.pNext            = NULL;                  // No allocator
            info.flags            = 0;                     // Must be 0
            info.image            = swapchainImages[i];    // Image
            info.viewType         = VK_IMAGE_VIEW_TYPE_2D; // Image is 2D
            info.format           = surfaceFormat.format;  // Image format
            info.components       = components;            // No swizzling (remapping)
            info.subresourceRange = subresourceRange;      // Content of image

            const VkResult result =
                vkCreateImageView(
                    logicalDevice,            // [in]  Logical device.
                    &info,                    // [in]  Image view params
                    NULL,                     // [in]  Allocator
                    &swapchainImageViews[i]); // [out] Image view handle

            if (result != VK_SUCCESS)
            {
                std::cerr << __FUNCTION__
                          << ": swap chain image view creation failed as "
                          << vk::stringify::result(result)
                          << std::endl;
                return false;
            }
        }

        return true;
    }

    void destroy()
    {
        for (size_t i = 0; i < swapchainImageViews.size(); i++)
            vkDestroyImageView(
                logicalDevice,          // [in] logical device handle
                swapchainImageViews[i], // [in] image view handle
                NULL);                  // [in] allocatior

        vkDestroySwapchainKHR(
            logicalDevice,      // [in] logical device handle
            swapchain,          // [in] swapchain handle
            nullptr);           // [in] allocator

        swapchainImageViews.clear();
        swapchain = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return swapchain != VK_NULL_HANDLE;
    }

    VkSurfaceKHR surface;
    VkDevice logicalDevice;
    VkRenderPass renderPass;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D imageExtent;
    uint32_t imageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
    std::vector<uint32_t> queueIndices;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
};

Swapchain::Swapchain(const VkDevice& logicalDevice,
                     const VkSurfaceKHR& surface)
    : impl(std::make_shared<Impl>(logicalDevice, surface))
{}

Swapchain& Swapchain::setSurfaceFormat(const VkSurfaceFormatKHR& surfaceFormat)
{
    impl->surfaceFormat = surfaceFormat;
    return *this;
}

VkSurfaceFormatKHR Swapchain::surfaceFormat() const
{ return impl->surfaceFormat; }

Swapchain& Swapchain::setPresentMode(const VkPresentModeKHR& presentMode)
{
    impl->presentMode = presentMode;
    return *this;
}

VkPresentModeKHR Swapchain::presentMode() const
{ return impl->presentMode; }

Swapchain& Swapchain::setImageExtent(const VkExtent2D& extent)
{
    impl->imageExtent = extent;
    return *this;
}

VkExtent2D Swapchain::imageExtent() const
{ return impl->imageExtent; }

Swapchain& Swapchain::setImageCount(const uint32_t imageCount)
{
    impl->imageCount = imageCount;
    return *this;
}

uint32_t Swapchain::imageCount() const
{ return impl->imageCount; }

Swapchain& Swapchain::setPreTransform(VkSurfaceTransformFlagBitsKHR preTransform)
{
    impl->preTransform = preTransform;
    return *this;
}

VkSurfaceTransformFlagBitsKHR Swapchain::preTransform() const
{ return impl->preTransform; }

Swapchain& Swapchain::setQueueIndicies(const std::vector<uint32_t>& indices)
{
    impl->queueIndices = indices;
    return *this;
}

std::vector<uint32_t> Swapchain::queueIndices() const
{ return impl->queueIndices; }

bool Swapchain::create()
{
    if (!isValid())
        return impl->create();
    return true;
}

void Swapchain::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Swapchain::isValid() const
{ return impl->isValid(); }

VkSwapchainKHR Swapchain::handle() const
{ return impl->swapchain; }

std::vector<VkImageView> Swapchain::imageViews() const
{ return impl->swapchainImageViews; }

} // namespace vk_capabilities
} // namespace kuu

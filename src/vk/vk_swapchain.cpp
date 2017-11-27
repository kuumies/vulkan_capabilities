/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Swapchain class
 * -------------------------------------------------------------------------- */

#include "vk_swapchain.h"
#include <algorithm>
#include <iostream>
#include "vk_stringify.h"

/* -------------------------------------------------------------------------- */

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- *
   Implementation of the swap chain.
 * -------------------------------------------------------------------------- */
struct Swapchain::Impl
{
    /* ----------------------------------------------------------------------- *
        Destroys the swapchain.
     * ----------------------------------------------------------------------- */
    ~Impl()
    {
        destroy();
    }

    /* ----------------------------------------------------------------------- *
        Creates the swapchain.
     * ----------------------------------------------------------------------- */
    void create()
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

        const VkResult result =
            vkCreateSwapchainKHR(
                logicalDevice, // [in]  logical device.
                &info,         // [in]  info
                nullptr,       // [in]  allocator
                &swapchain);   // [out] swapchain handle
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": swap chain creation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return;
        }
    }
    /* ----------------------------------------------------------------------- *
        Destroys the swapchain.
     * ----------------------------------------------------------------------- */
    void destroy()
    {
        vkDestroySwapchainKHR(
            logicalDevice,      // [in] logical device handle
            swapchain,          // [in] swapchain handle
            nullptr);           // [in] allocator
    }

    VkSurfaceKHR surface;
    VkDevice logicalDevice;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    VkExtent2D imageExtent;
    uint32_t imageCount;
    VkSurfaceTransformFlagBitsKHR preTransform;
    std::vector<uint32_t> queueIndices;
    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
};

/* -------------------------------------------------------------------------- *
   Constructs the swap chain instance.
 * -------------------------------------------------------------------------- */
Swapchain::Swapchain(const VkSurfaceKHR& surface,
                     const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->surface       = surface;
    impl->logicalDevice = logicalDevice;
}

/* -------------------------------------------------------------------------- *
   Sets the swap chain format.
 * -------------------------------------------------------------------------- */
Swapchain& Swapchain::setSurfaceFormat(const VkSurfaceFormatKHR& surfaceFormat)
{
    impl->surfaceFormat = surfaceFormat;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the swap chain format.
 * -------------------------------------------------------------------------- */
VkSurfaceFormatKHR Swapchain::surfaceFormat() const
{ return impl->surfaceFormat; }

/* -------------------------------------------------------------------------- *
   Sets the present mode.
 * -------------------------------------------------------------------------- */
Swapchain& Swapchain::setPresentMode(const VkPresentModeKHR& presentMode)
{
    impl->presentMode = presentMode;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the present mode.
 * -------------------------------------------------------------------------- */
VkPresentModeKHR Swapchain::presentMode() const
{ return impl->presentMode; }

/* -------------------------------------------------------------------------- *
   Sets the image extent.
 * -------------------------------------------------------------------------- */
Swapchain& Swapchain::setImageExtent(const VkExtent2D& extent)
{
    impl->imageExtent = extent;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the image extent.
 * -------------------------------------------------------------------------- */
VkExtent2D Swapchain::imageExtent() const
{ return impl->imageExtent; }

/* -------------------------------------------------------------------------- *
   Sets the image count.
 * -------------------------------------------------------------------------- */
Swapchain& Swapchain::setImageCount(const uint32_t imageCount)
{
    impl->imageCount = imageCount;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the image count.
 * -------------------------------------------------------------------------- */
uint32_t Swapchain::imageCount() const
{ return impl->imageCount; }

/* -------------------------------------------------------------------------- *
   Sets the pre-transform.
 * -------------------------------------------------------------------------- */
Swapchain& Swapchain::setPreTransform(VkSurfaceTransformFlagBitsKHR preTransform)
{
    impl->preTransform = preTransform;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the pre-transform.
 * -------------------------------------------------------------------------- */
VkSurfaceTransformFlagBitsKHR Swapchain::preTransform() const
{ return impl->preTransform; }

/* -------------------------------------------------------------------------- *
   Sets the queue indices.
 * -------------------------------------------------------------------------- */
Swapchain& Swapchain::setQueueIndicies(const std::vector<uint32_t>& indices)
{
    impl->queueIndices = indices;
    return *this;
}

/* -------------------------------------------------------------------------- *
   Returns the queue indices.
 * -------------------------------------------------------------------------- */
std::vector<uint32_t> Swapchain::queueIndices() const
{ return impl->queueIndices; }

/* -------------------------------------------------------------------------- *
   Creates the swap chain.
 * -------------------------------------------------------------------------- */
void Swapchain::create()
{
    if (!isValid())
        impl->create();
}

/* -------------------------------------------------------------------------- *
   Destroys the swap chain.
 * -------------------------------------------------------------------------- */
void Swapchain::destroy()
{
    if (isValid())
        impl->destroy();
}

/* -------------------------------------------------------------------------- *
   Returns true if the swap chain is not a null handle.
 * -------------------------------------------------------------------------- */
bool Swapchain::isValid() const
{ return impl->swapchain != VK_NULL_HANDLE; }

} // namespace vk_capabilities
} // namespace kuu

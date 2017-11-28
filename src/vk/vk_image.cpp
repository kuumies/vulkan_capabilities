/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Image class
 * -------------------------------------------------------------------------- */

#include "vk_image.h"
#include <algorithm>
#include <iostream>
#include "vk_stringify.h"
#include "vk_helper.h"

namespace kuu
{
namespace vk
{

struct Image::Impl
{
    ~Impl()
    {
        destroy();
    }

    void create()
    {
        // Fill create info
        VkStructureType structType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        VkImageCreateInfo info;
        info.sType                 = structType;                // Must be this type.
        info.pNext                 = NULL;                      // No extensions in use
        info.flags                 = 0;                         // No flags, TODO: get from user
        info.imageType             = type;                      // Image type
        info.format                = format;                    // Image format
        info.extent                = extent;                    // Image extent
        info.mipLevels             = 1;                         // No mipmap levels, TODO: get from user
        info.arrayLayers           = 1;                         // No array layers, TODO: get from user
        info.samples               = VK_SAMPLE_COUNT_1_BIT;     // No multisampling, TODO: get from user
        info.tiling                = tiling;                    // Image tiling
        info.usage                 = usage;                     // Image usage
        info.sharingMode           = VK_SHARING_MODE_EXCLUSIVE; // No sharing
        info.queueFamilyIndexCount = 0,                         // No sharing
        info.pQueueFamilyIndices   = NULL;                      // No sharing
        info.initialLayout         = initialLayout;             // Image layout

        // Create image
        VkResult result =
            vkCreateImage(
                logicalDevice,
                &info,
                NULL,
                &image);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": image creation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return;
        }

        // Create physical device memory properties
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(
            physicalDevice,     // [in]  physical device handle
            &memoryProperties); // [out] memory properties

        // Create image memory requirements
        VkMemoryRequirements mememoryRequirements;
        vkGetImageMemoryRequirements(
            logicalDevice,     // [in] logical device
            image,             // [in]  image
            &mememoryRequirements); // [out] memory requirements

        // Create memory allocate info.
        VkMemoryAllocateInfo memmoryAllocate = {};
        memmoryAllocate.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memmoryAllocate.allocationSize  = mememoryRequirements.size;
        memmoryAllocate.memoryTypeIndex =
            helper::findMemoryTypeIndex(
                memoryProperties,
                mememoryRequirements,
                memoryProperty);

        // Allocate image memory
        result = vkAllocateMemory(
            logicalDevice, // [in]  logical device handle
            &memmoryAllocate,     // [in]  memory allocation info
            NULL,          // [in]  custom allocator
            &memory);      // [out] memory handle

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": image memory allocation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return;
        }

        // Bind image memory.
        result = vkBindImageMemory(
            logicalDevice, // [in] logical device handle
            image,         // [in] image handle
            memory,        // [in] memory handle
            0);            // [in] memory offset

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": image memory bind failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return;
        }

        // Create view (see vk_swapchain.cpp)
        VkStructureType viewType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        VkImageViewCreateInfo viewInfo;
        viewInfo.sType    = viewType;
        viewInfo.pNext    = NULL;
        viewInfo.flags    = 0;
        viewInfo.image    = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format   = format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewInfo.subresourceRange.aspectMask     = viewAspectMask;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        result =
            vkCreateImageView(
                logicalDevice, // [in]  logical device handle
                &viewInfo,     // [in]  image view create info
                NULL,          // [in]  custom allocator
                &imageView);   // [out] image view handle

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": image view creation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return;
        }
    }

    void destroy()
    {
        // Destroys the image view
        vkDestroyImageView(
            logicalDevice,  // [in] logical device handle
            imageView,      // [in] image view handle
            NULL);          // [in] custom allocator

        // Destroys image
        vkDestroyImage(
            logicalDevice, // [in] logical device handle
            image,         // [in] image handle
            NULL);         // [in] custom allocator

        // Free memory
        vkFreeMemory(
            logicalDevice,  // [in] logical device handle
            memory,         // [in] memory handle
            NULL);          // [in] custom allocator

        // Reset handles
        image     = VK_NULL_HANDLE;
        imageView = VK_NULL_HANDLE;
        memory    = VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;

    // User image values
    VkImageType type            = VK_IMAGE_TYPE_2D;
    VkFormat format;
    VkExtent3D extent;
    VkImageTiling tiling        = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // User image view values
    VkImageAspectFlags viewAspectMask;

    // User memory values
    VkMemoryPropertyFlags memoryProperty;

    // Handles
    VkImage image         = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

Image::Image(const VkPhysicalDevice& physicalDevice,
             const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice  = logicalDevice;
    impl->physicalDevice = physicalDevice;
}

Image& Image::setType(VkImageType type)
{
    impl->type = type;
    return *this;
}

VkImageType Image::type() const
{ return impl->type; }

Image &Image::setFormat(VkFormat format)
{
    impl->format = format;
    return *this;
}

VkFormat Image::format() const
{ return impl->format; }

Image& Image::setExtent(VkExtent3D extent)
{
    impl->extent = extent;
    return *this;
}

VkExtent3D Image::extent() const
{ return impl->extent; }

Image& Image::setTiling(VkImageTiling tiling)
{
    impl->tiling = tiling;
    return *this;
}

VkImageTiling Image::tiling() const
{ return impl->tiling; }

Image& Image::setUsage(VkImageUsageFlags usage)
{
    impl->usage = usage;
    return *this;
}

VkImageUsageFlags Image::usage() const
{ return impl->usage; }

Image& Image::setInitialLayout(VkImageLayout layout)
{
    impl->initialLayout = layout;
    return *this;
}

VkImageLayout Image::initialLayout() const
{ return impl->initialLayout; }

Image& Image::setImageViewAspect(VkImageAspectFlags aspect)
{
    impl->viewAspectMask = aspect;
    return *this;
}

VkImageAspectFlags Image::imageViewAspectlayout() const
{ return impl->viewAspectMask; }

Image& Image::setMemoryProperty(VkMemoryPropertyFlags property)
{
    impl->memoryProperty = property;
    return *this;
}

VkMemoryPropertyFlags Image::memoryProperty() const
{ return impl->memoryProperty; }

void Image::create()
{
    if (!isValid())
        impl->create();
}

void Image::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Image::isValid() const
{ return impl->image != VK_NULL_HANDLE; }

VkImage Image::imageHandle() const
{ return impl->image; }

VkImageView Image::imageViewHandle() const
{ return impl->imageView; }

} // namespace vk
} // namespace kuu

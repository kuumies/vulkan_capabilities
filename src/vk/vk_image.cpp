/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Image class
 * -------------------------------------------------------------------------- */

#include "vk_image.h"
#include <algorithm>
#include <iostream>
#include "vk_command.h"
#include "vk_buffer.h"
#include "vk_helper.h"
#include "vk_queue.h"
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

struct Sampler::Impl
{
    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 12.0f;

        // Create image
        const VkResult result =
            vkCreateSampler(
                logicalDevice,
                &samplerInfo,
                NULL,
                &sampler);

        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": sampler creation failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        return true;
    }

    void destroy()
    {
        vkDestroySampler(
            logicalDevice,
            sampler,
            NULL);

        sampler = VK_NULL_HANDLE;
    }

    bool isValid() const
    {
        return sampler != VK_NULL_HANDLE;
    }

    // Parent
    VkDevice logicalDevice = VK_NULL_HANDLE;

    // Child
    VkSampler sampler = VK_NULL_HANDLE;
};

/* -------------------------------------------------------------------------- */

Sampler::Sampler(const VkDevice& logicalDevice)
    : impl(std::make_shared<Impl>())
{
    impl->logicalDevice = logicalDevice;
}

Sampler& Sampler::setDevice(const VkDevice& logicalDevice)
{
    impl->logicalDevice = logicalDevice;
    return *this;
}

VkDevice Sampler::device() const
{ return impl->logicalDevice; }

bool Sampler::create()
{
    if (!impl->isValid())
        return impl->create();
    return true;
}

void Sampler::destroy()
{
    if (impl->isValid())
        impl->destroy();
}

bool Sampler::isValid() const
{ return impl->isValid(); }

VkSampler Sampler::handle() const
{ return impl->sampler; }

/* -------------------------------------------------------------------------- */

struct Image::Impl
{
    ~Impl()
    {
        if (isValid())
            destroy();
    }

    bool create()
    {
        uint32_t mipLevels = 1;
        if (generateMipmaps)
            mipLevels = uint32_t(std::floor(std::log2(std::max(extent.width, extent.height)))) + 1;
        mipLevelCount = mipLevels;

        // Fill create info
        VkStructureType structType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        VkImageCreateInfo info;
        info.sType                 = structType;                // Must be this type.
        info.pNext                 = NULL;                      // No extensions in use
        info.flags                 = 0;                         // No flags, TODO: get from user
        info.imageType             = type;                      // Image type
        info.format                = format;                    // Image format
        info.extent                = extent;                    // Image extent
        info.mipLevels             = mipLevels;                 // Mipmap levels
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
            return false;
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
            return false;
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
            return false;
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
        viewInfo.subresourceRange.levelCount     = mipLevels;
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
            return false;
        }

        if (generateMipmaps)
        {

        }

        return true;
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

    bool isValid() const
    {
        return image != VK_NULL_HANDLE;
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

    // User sampler
    Sampler sampler;

    // Generate mip maps
    bool generateMipmaps = false;
    uint32_t mipLevelCount = 1;

    // Handles
    VkImage image         = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
};

/* -------------------------------------------------------------------------- */

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

Image& Image::setSampler(const Sampler& sampler)
{
    impl->sampler = sampler;
    return *this;
}

Sampler Image::sampler() const
{ return impl->sampler; }

Image& Image::setGenerateMipLevels(bool generate)
{
    impl->generateMipmaps = generate;
    return *this;
}

bool  Image::create()
{
    if (!isValid())
        return impl->create();
    return false;
}

void Image::destroy()
{
    if (isValid())
        impl->destroy();
}

bool Image::isValid() const
{ return impl->isValid(); }

VkImage Image::imageHandle() const
{ return impl->image; }

VkImageView Image::imageViewHandle() const
{ return impl->imageView; }

bool Image::transitionLayout(
    const VkImageLayout& oldLayout,
    const VkImageLayout& newLayout,
    const VkPipelineStageFlags srcStageMask,
    const VkPipelineStageFlags dstStageMask,
    Queue& queue,
    CommandPool& commandPool,
    const VkImageAspectFlagBits imageAspect,
    const uint32_t mipLevel)
{
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    // Source layouts (old)
    // Source access mask controls actions that have to be finished on the old layout
    // before it will be transitioned to the new layout
    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source
        // Make sure any reads from the image have been finished
        srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        dstAccessMask = dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (srcAccessMask == 0)
        {
            srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        std::cerr << __FUNCTION__
                  << ": image layout transition is not supported"
                  << std::endl;
        return false;
    }

    VkCommandBuffer cmdBuf = commandPool.allocateBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    VkImageMemoryBarrier barrier = {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext                           = NULL;
    barrier.srcAccessMask                   = srcAccessMask;
    barrier.dstAccessMask                   = dstAccessMask;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = impl->image;
    barrier.subresourceRange.aspectMask     = imageAspect;
    barrier.subresourceRange.baseMipLevel   = mipLevel;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    vkCmdPipelineBarrier(
        cmdBuf,
        srcStageMask,
        dstStageMask,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    const VkResult result = vkEndCommandBuffer(cmdBuf);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": image layout transition failed as "
                  << vk::stringify::result(result)
                  << std::endl;
        return false;
    }

    if (!queue.submit(cmdBuf))
        return false;
    if (!queue.waitIdle())
        return false;

    impl->initialLayout = newLayout;
    return true;
}

bool Image::copyFromBuffer(
    const Buffer& buffer,
    Queue& queue,
    CommandPool& commandPool,
    const std::vector<VkBufferImageCopy>& regions)
{
    std::vector<VkBufferImageCopy> usedRegions = regions;
    if (usedRegions.size() == 0)
    {
        VkBufferImageCopy region;
        region.bufferOffset      = 0;
        region.bufferRowLength   = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = impl->extent;

        usedRegions.push_back(region);
    }

    VkCommandBuffer cmdBuf =
        commandPool.allocateBuffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    vkCmdCopyBufferToImage(
        cmdBuf,
        buffer.handle(),
        impl->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        uint32_t(usedRegions.size()),
        usedRegions.data());

    const VkResult result = vkEndCommandBuffer(cmdBuf);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": image copy from buffer failed as "
                  << vk::stringify::result(result)
                  << std::endl;
        return false;
    }

    if (!queue.submit(cmdBuf))
        return false;
    if (!queue.waitIdle())
        return false;

    return true;
}

bool Image::generateMipLevels(Queue& queue, CommandPool& commandPool)
{
    for (uint32_t i = 1 ; i <impl->mipLevelCount; ++i)
    {
        VkImageBlit imageBlit = {};

        std::cout << i << ", " << impl->extent.width << ", " << (impl->extent.width >> i) << std::endl;

        // Source
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel = i-1;
        imageBlit.srcOffsets[1].x = int32_t(impl->extent.width >> (i - 1));
        imageBlit.srcOffsets[1].y = int32_t(impl->extent.height >> (i - 1));
        imageBlit.srcOffsets[1].z = 1;

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstOffsets[1].x = int32_t(impl->extent.width >> i);
        imageBlit.dstOffsets[1].y = int32_t(impl->extent.height >> i);
        imageBlit.dstOffsets[1].z = 1;

        transitionLayout(VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         queue,
                         commandPool,
                         VK_IMAGE_ASPECT_COLOR_BIT,
                         i);

        VkCommandBuffer cmdBuf =
            commandPool.allocateBuffer(
                VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuf, &beginInfo);
        vkCmdBlitImage(
          cmdBuf,
          impl->image,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          impl->image,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          1,
          &imageBlit,
          VK_FILTER_LINEAR);

        const VkResult result = vkEndCommandBuffer(cmdBuf);
        if (result != VK_SUCCESS)
        {
            std::cerr << __FUNCTION__
                      << ": image copy from buffer failed as "
                      << vk::stringify::result(result)
                      << std::endl;
            return false;
        }

        if (!queue.submit(cmdBuf))
            return false;
        if (!queue.waitIdle())
            return false;

        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         queue,
                         commandPool,
                         VK_IMAGE_ASPECT_COLOR_BIT,
                         i);
    }

    return true;
}

} // namespace vk
} // namespace kuu

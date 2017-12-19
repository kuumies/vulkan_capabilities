/* -------------------------------------------------------------------------- *
   Antti Jumpponen <kuumies@gmail.com>
   The implementation of kuu::vk::Texture2D struct.
 * -------------------------------------------------------------------------- */

#include "vk_texture.h"
#include <iostream>
#include <QtCore/QTime>
#include <QtGui/QImage>
#include "vk_buffer.h"
#include "vk_command.h"
#include "vk_helper.h"
#include "vk_queue.h"
#include "vk_stringify.h"

namespace kuu
{
namespace vk
{

/* -------------------------------------------------------------------------- */

VkImage createImage(const VkDevice& device,
                    const VkFormat& format,
                    const VkExtent3D& extent,
                    const uint32_t& mipLevels,
                    const uint32_t& arrayLayers,
                    VkImageCreateFlags flags,
                    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                                              VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                              VK_IMAGE_USAGE_SAMPLED_BIT)
{
    // Create image.
    VkImageCreateInfo info = {};
    info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType     = VK_IMAGE_TYPE_2D;
    info.format        = format;
    info.extent        = extent;
    info.mipLevels     = mipLevels;
    info.arrayLayers   = arrayLayers;
    info.samples       = VK_SAMPLE_COUNT_1_BIT;
    info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    info.usage         = usage;
    info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.flags         = flags;

    // Create image
    VkImage image;
    VkResult result =
        vkCreateImage(
            device,
            &info,
            NULL,
            &image);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": image creation failed as "
                  << vk::stringify::result(result)
                  << std::endl;
        return VK_NULL_HANDLE;
    }

    return image;
}

VkDeviceMemory allocateMemory(const VkPhysicalDevice& physicalDevice,
                              const VkDevice& device,
                              const VkImage& image)
{
    // Create physical device memory properties
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(
        physicalDevice,
        &memoryProperties);

    // Create image memory requirements
    VkMemoryRequirements mememoryRequirements;
    vkGetImageMemoryRequirements(
        device,
        image,
        &mememoryRequirements);

    // Create memory allocate info.
    VkMemoryAllocateInfo memmoryAllocate = {};
    memmoryAllocate.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memmoryAllocate.allocationSize  = mememoryRequirements.size;
    memmoryAllocate.memoryTypeIndex =
        helper::findMemoryTypeIndex(
            memoryProperties,
            mememoryRequirements,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    // Allocate image memory
    VkDeviceMemory memory;
    VkResult result = vkAllocateMemory(
        device,
        &memmoryAllocate,
        NULL,
        &memory);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": image memory allocation failed as "
                  << vk::stringify::result(result)
                  << std::endl;
        return VK_NULL_HANDLE;
    }

    // Bind image memory.
    result = vkBindImageMemory(
        device,
        image,
        memory,
        0);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": image memory bind failed as "
                  << vk::stringify::result(result)
                  << std::endl;
        return VK_NULL_HANDLE;
    }

    return memory;
}

VkImageView createImageView(const VkDevice& device,
                            const VkImage& image,
                            const VkFormat& format,
                            const uint32_t& mipLevels,
                            const VkImageViewType& viewType,
                            const uint32_t& layers,
                            const VkImageAspectFlags aspectFlags =
                                VK_IMAGE_ASPECT_COLOR_BIT)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image    = image;
    viewInfo.viewType = viewType;
    viewInfo.format   = format;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.subresourceRange = {};
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.layerCount = layers;

    VkImageView imageView;
    const VkResult result =
        vkCreateImageView(
            device,
            &viewInfo,
            NULL,
            &imageView);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": image view creation failed as "
                  << vk::stringify::result(result)
                  << std::endl;
        return VK_NULL_HANDLE;
    }

    return imageView;
}

VkSampler createSampler(const VkDevice& device,
                        const VkFilter& magFilter,
                        const VkFilter& minFilter,
                        const VkSamplerAddressMode& addressModeU,
                        const VkSamplerAddressMode& addressModeV,
                        const VkSamplerAddressMode& addressModeW,
                        const uint32_t& mipLevels)
{
    VkSamplerCreateInfo info = {};
    info.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter               = magFilter;
    info.minFilter               = minFilter;
    info.addressModeU            = addressModeU;
    info.addressModeV            = addressModeV;
    info.addressModeW            = addressModeW;
    info.anisotropyEnable        = VK_TRUE;
    info.maxAnisotropy           = 16;
    info.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    info.unnormalizedCoordinates = VK_FALSE;
    info.compareEnable           = VK_FALSE;
    info.compareOp               = VK_COMPARE_OP_ALWAYS;
    info.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.mipLodBias              = 0.0f;
    info.minLod                  = 0.0f;
    info.maxLod                  = float(mipLevels);

    VkSampler sampler;
    const VkResult result =
        vkCreateSampler(
            device,
            &info,
            NULL,
            &sampler);

    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": sampler creation failed as "
                  << vk::stringify::result(result)
                  << std::endl;
        return VK_NULL_HANDLE;
    }

    return sampler;
}

bool commandTransitionImageLayout(
    const VkImage& image,
    const VkImageLayout& oldLayout,
    const VkImageLayout& newLayout,
    const VkPipelineStageFlags srcStageMask,
    const VkPipelineStageFlags dstStageMask,
    const uint32_t mipLevel,
    const uint32_t layerCount,
    const VkCommandBuffer& cmdBuf)
{
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;

    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    default:
        std::cerr << __FUNCTION__
                  << ": old image layout transition is not supported"
                  << std::endl;
        return false;
    }

    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        if (srcAccessMask == 0)
            srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;

    default:
        std::cerr << __FUNCTION__
                  << ": new image layout transition is not supported"
                  << std::endl;
        return false;
    }

    VkImageMemoryBarrier barrier = {};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.pNext                           = NULL;
    barrier.srcAccessMask                   = srcAccessMask;
    barrier.dstAccessMask                   = dstAccessMask;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = mipLevel;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = layerCount;

    vkCmdPipelineBarrier(
        cmdBuf,
        srcStageMask,
        dstStageMask,
        0,
        0, NULL,
        0, NULL,
        1, &barrier
    );

    return true;
}

void commandCopyBufferToImage(
    const VkBuffer& buffer,
    const VkImage& image,
    const VkCommandBuffer& cmdBuf,
    const std::vector<VkBufferImageCopy>& usedRegions)
{
    vkCmdCopyBufferToImage(
        cmdBuf,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        uint32_t(usedRegions.size()),
        usedRegions.data());
}

void commandCopyBufferToImage(
    const VkBuffer& buffer,
    const VkImage& image,
    const VkExtent3D& extent,
    const VkCommandBuffer& cmdBuf)
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
    region.imageExtent = extent;

    std::vector<VkBufferImageCopy> usedRegions;
    usedRegions.push_back(region);

    commandCopyBufferToImage(buffer, image, cmdBuf, usedRegions);
}

void recordCommands(
    const VkCommandBuffer& cmdBuf,
    const VkImage& image,
    const VkBuffer& imageDataBuffer,
    const VkExtent3D& extent,
    const uint32_t mipmapCount)
{
    // Transition image into transfer destination layout
    commandTransitionImageLayout(
        image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0,
        1,
        cmdBuf);

    // Copy buffer to image.
    commandCopyBufferToImage(imageDataBuffer, image, extent, cmdBuf);

    // Transition image into transfer source layout
    commandTransitionImageLayout(
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0, 1, cmdBuf);

    for (uint32_t i = 1 ; i < mipmapCount; ++i)
    {
        VkImageBlit imageBlit = {};

        // Source
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel = i-1;
        imageBlit.srcOffsets[1].x = int32_t(extent.width >> (i - 1));
        imageBlit.srcOffsets[1].y = int32_t(extent.height >> (i - 1));
        imageBlit.srcOffsets[1].z = 1;

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstOffsets[1].x = int32_t(extent.width >> i);
        imageBlit.dstOffsets[1].y = int32_t(extent.height >> i);
        imageBlit.dstOffsets[1].z = 1;

        commandTransitionImageLayout(
            image,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            i, 1, cmdBuf);

        vkCmdBlitImage(
          cmdBuf,
          image,
          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
          image,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          1,
          &imageBlit,
          VK_FILTER_LINEAR);

        commandTransitionImageLayout(
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            i, 1, cmdBuf);
    }

    // Transition image into optimal shader read layout.
    commandTransitionImageLayout(
        image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 1, cmdBuf);
}

bool textureFromImage(const VkDevice& device,
                      const VkPhysicalDevice& physicalDevice,
                      const QImage& img,
                      const VkFormat& format,
                      const VkCommandBuffer& cmdBuf,
                      const bool generateMipmaps,
                      const VkFilter magFilter,
                      const VkFilter minFilter,
                      const VkSamplerAddressMode addressModeU,
                      const VkSamplerAddressMode addressModeV,
                      std::shared_ptr<Buffer>& imageDataBuffer,
                      VkImage& image,
                      VkImageView& imageView,
                      VkSampler& sampler,
                      VkDeviceMemory& memory)
{
    VkExtent3D extent = { uint32_t(img.width()), uint32_t(img.height()), 1 };

    // Calc. mipmap count.
    uint32_t mipmapCount = 1;
    if (generateMipmaps)
        mipmapCount = uint32_t(std::floor(std::log2(std::max(img.width(), img.height())))) + 1;

    // Create image.
    image = createImage(
        device,
        format,
        extent,
        mipmapCount,
        1,
        0);
    if (image == VK_NULL_HANDLE)
        return false;

    // Allocate memory.
    memory = allocateMemory(physicalDevice, device, image);
    if (memory == VK_NULL_HANDLE)
        return false;

    // Create view.
    imageView = createImageView(device, image, format, mipmapCount, VK_IMAGE_VIEW_TYPE_2D, 1);
    if (imageView == VK_NULL_HANDLE)
        return false;

    // Create sampler.
    sampler = createSampler(device, magFilter, minFilter, addressModeU, addressModeV, VK_SAMPLER_ADDRESS_MODE_REPEAT, mipmapCount);
    if (sampler == VK_NULL_HANDLE)
        return false;

    // Copy pixels from image into host local buffer.
    const VkDeviceSize imageSize = img.byteCount();
    imageDataBuffer = std::make_shared<Buffer>(physicalDevice, device);
    imageDataBuffer->setSize(imageSize);
    imageDataBuffer->setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    imageDataBuffer->setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!imageDataBuffer->create())
        return false;
    imageDataBuffer->copyHostVisible(img.bits(), imageSize);

    // Record commands
    recordCommands(cmdBuf, image, imageDataBuffer->handle(), extent, mipmapCount);

    return true;
}

/* -------------------------------------------------------------------------- */

struct Texture2D::Impl
{
    Impl(const VkDevice& device, Texture2D* self)
        : device(device)
        , self(self)
    {}

    ~Impl()
    {
        vkDestroySampler(device, self->sampler, NULL);
        vkDestroyImageView(device,  self->imageView, NULL);
        vkDestroyImage(device,  self->image, NULL);
        vkFreeMemory(device, self->memory, NULL);
    }

    VkDevice device;
    Texture2D* self;
};

/* -------------------------------------------------------------------------- */

Texture2D::Texture2D(const VkDevice& device)
    : format(VK_FORMAT_UNDEFINED)
    , image(VK_NULL_HANDLE)
    , imageView(VK_NULL_HANDLE)
    , sampler(VK_NULL_HANDLE)
    , impl(std::make_shared<Impl>(device, this))
{}

Texture2D::Texture2D(const VkPhysicalDevice& physicalDevice,
                     const VkDevice& device,
                     Queue& queue,
                     CommandPool& commandPool,
                     const std::string& filePath,
                     VkFilter magFilter,
                     VkFilter minFilter,
                     VkSamplerAddressMode addressModeU,
                     VkSamplerAddressMode addressModeV,
                     bool generateMipmaps)
    : format(VK_FORMAT_UNDEFINED)
    , image(VK_NULL_HANDLE)
    , imageView(VK_NULL_HANDLE)
    , sampler(VK_NULL_HANDLE)
    , impl(std::make_shared<Impl>(device, this))
{
    // Load image
    QImage img(QString::fromStdString(filePath));
    if (!img.isGrayscale())
    {
        img = img.convertToFormat(QImage::Format_RGB32);
        img = img.rgbSwapped();
        if (img.isNull())
        {
            std::cerr << __FUNCTION__
                      << ": image is not a RGB or RGBA image "
                      << filePath
                      << std::endl;
            return;
        }
    }
    else
    {
        img = img.convertToFormat(QImage::Format_Grayscale8);
    }

    // Set the image format
    format = img.isGrayscale() ? VK_FORMAT_R8_UNORM
                               : VK_FORMAT_R8G8B8A8_UNORM;
    // Set the image extent
    extent = { uint32_t(img.width()), uint32_t(img.height()) };

    // Allocate buffer for queue commands.
    VkCommandBuffer cmdBuf =
        commandPool.allocateBuffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Start recording commands
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    // Create texture and record commands
    std::shared_ptr<Buffer> imageDataBuffer;
    textureFromImage(device,
                     physicalDevice,
                     img,
                     format,
                     cmdBuf,
                     generateMipmaps,
                     magFilter,
                     minFilter,
                     addressModeU,
                     addressModeV,
                     imageDataBuffer,
                     image,
                     imageView,
                     sampler,
                     memory);

    // Stop recording commands.
    const VkResult result = vkEndCommandBuffer(cmdBuf);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to apply image commands as "
                  << vk::stringify::result(result)
                  << std::endl;
        return;
    }

    // Submit commands into queue
    if (!queue.submit(cmdBuf))
        return;

    // Wait until commands has been processed.
    if (!queue.waitIdle())
        return;
}

Texture2D::Texture2D(const VkPhysicalDevice& physicalDevice,
                     const VkDevice& device,
                     const VkExtent2D& extent,
                     const VkFormat& format,
                     VkFilter magFilter,
                     VkFilter minFilter,
                     VkSamplerAddressMode addressModeU,
                     VkSamplerAddressMode addressModeV,
                     VkImageUsageFlags usageFlags,
                     VkImageAspectFlags aspectFlags)
    : format(format)
    , extent(extent)
    , image(VK_NULL_HANDLE)
    , imageView(VK_NULL_HANDLE)
    , sampler(VK_NULL_HANDLE)
    , impl(std::make_shared<Impl>(device, this))
{
    const uint32_t mipmapCount = 1;

    // Create image.
    VkExtent3D imageExtent = { extent.width, extent.height, 1 };
    image = createImage(
        device,
        format,
        imageExtent,
        mipmapCount,
        1,
        0,
        usageFlags);
    if (image == VK_NULL_HANDLE)
        return;

    // Allocate memory.
    memory = allocateMemory(physicalDevice, device, image);
    if (memory == VK_NULL_HANDLE)
        return;

    // Create view.
    imageView = createImageView(device, image, format, mipmapCount, VK_IMAGE_VIEW_TYPE_2D, 1, aspectFlags);
    if (imageView == VK_NULL_HANDLE)
        return;

    // Create sampler.
    sampler = createSampler(device, magFilter, minFilter, addressModeU, addressModeV, VK_SAMPLER_ADDRESS_MODE_REPEAT, mipmapCount);
    if (sampler == VK_NULL_HANDLE)
        return;
}

/* -------------------------------------------------------------------------- */

std::map<std::string, std::shared_ptr<Texture2D>>
    loadtextures(std::vector<std::string> filepaths,
                 const VkPhysicalDevice& physicalDevice,
                 const VkDevice& device,
                 Queue& queue,
                 CommandPool& commandPool,
                 VkFilter magFilter,
                 VkFilter minFilter,
                 VkSamplerAddressMode addressModeU,
                 VkSamplerAddressMode addressModeV,
                 bool generateMipmaps)
{
    // Load only imges with an unique paths
    std::sort(filepaths.begin(), filepaths.end());
    filepaths.erase(std::unique(filepaths.begin(), filepaths.end() ), filepaths.end());

    // Load images asynchronously
    std::vector<QImage> images;
    images.resize(filepaths.size());

    #pragma omp parallel for
    for (int i = 0; i < filepaths.size(); ++i)
    {
        QImage img(QString::fromStdString(filepaths[i]));
        if (!img.isGrayscale())
        {
            img = img.convertToFormat(QImage::Format_ARGB32);
            img = img.rgbSwapped();
        }

        images[i] = img;
    }

    // Allocate buffers for queue commands.
    VkCommandBuffer cmdBuf =
        commandPool.allocateBuffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Start recording commands
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    // Buffers needs to be alive until commands are submitted into quueue.
    std::vector<std::shared_ptr<Buffer>> buffers;
    buffers.resize(filepaths.size());

    std::map<std::string, std::shared_ptr<Texture2D>> results;
    for (int f = 0; f < filepaths.size(); ++f)
    {
        std::shared_ptr<Texture2D> tex = std::make_shared<Texture2D>(device);
        tex->format = images[f].isGrayscale() ? VK_FORMAT_R8_UNORM
                                              : VK_FORMAT_R8G8B8A8_UNORM;
        textureFromImage(device,
                         physicalDevice,
                         images[f],
                         tex->format,
                         cmdBuf,
                         generateMipmaps,
                         magFilter,
                         minFilter,
                         addressModeU,
                         addressModeV,
                         buffers[f],
                         tex->image,
                         tex->imageView,
                         tex->sampler,
                         tex->memory);

        results[filepaths[f]] = tex;
    }

    // Stop recording commands.
    const VkResult result = vkEndCommandBuffer(cmdBuf);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to apply image commands as "
                  << vk::stringify::result(result)
                  << std::endl;
        return results;
    }

    // Submit commands into queue
    if (!queue.submit(cmdBuf))
        return results;

    // Wait until commands has been processed.
    if (!queue.waitIdle())
        return results;

    return results;
}

/* -------------------------------------------------------------------------- */

struct TextureCube::Impl
{
    Impl(const VkDevice& device, TextureCube* self)
        : device(device)
        , self(self)
    {}

    ~Impl()
    {
        vkDestroySampler(device,   self->sampler,   NULL);
        vkDestroyImageView(device, self->imageView, NULL);
        vkDestroyImage(device,     self->image,     NULL);
        vkFreeMemory(device,       self->memory,    NULL);
    }

    VkDevice device;
    TextureCube* self;
};

/* -------------------------------------------------------------------------- */

TextureCube::TextureCube(
        const VkPhysicalDevice& physicalDevice,
        const VkDevice& device,
        Queue& queue,
        CommandPool& commandPool,
        const std::vector<std::string>& filePaths,
        VkFilter magFilter,
        VkFilter minFilter,
        VkSamplerAddressMode addressModeU,
        VkSamplerAddressMode addressModeV,
        VkSamplerAddressMode addressModeW)
    : format(VK_FORMAT_UNDEFINED)
    , image(VK_NULL_HANDLE)
    , imageView(VK_NULL_HANDLE)
    , sampler(VK_NULL_HANDLE)
    , impl(std::make_shared<Impl>(device, this))
{
    if (filePaths.size() != 6)
        return;

    // Load images asynchronously
    std::vector<QImage> images;
    images.resize(filePaths.size());

    #pragma omp parallel for
    for (int i = 0; i < filePaths.size(); ++i)
    {
        QImage img(QString::fromStdString(filePaths[i]));
        img = img.convertToFormat(QImage::Format_ARGB32);
        img = img.rgbSwapped();
        images[i] = img;
    }

    uint32_t totalSize = 0;
    for (const QImage& image : images)
        totalSize += image.byteCount();

    // Create a single buffer for images data.
    Buffer buf(physicalDevice, device);
    buf.setSize(totalSize);
    buf.setUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    buf.setMemoryProperties(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (!buf.create())
        return;

    // Copy images into single buffer
    VkDeviceSize offset = 0;
    for (const QImage& image : images)
    {
        buf.copyHostVisible(image.bits(), image.byteCount(), offset);
        offset += image.byteCount();
    }

    // Images dimensions.
    VkExtent3D extent;
    extent.width  = images[0].width();
    extent.height = images[0].height();
    extent.depth  = 1;

    // Crate texture cube image
    format = VK_FORMAT_R8G8B8A8_UNORM;
    image = createImage(device,
                        format,
                        extent,
                        1,
                        6,
                        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);

    // Allocate memory.
    memory = allocateMemory(physicalDevice, device, image);
    if (memory == VK_NULL_HANDLE)
        return;

    // Create view.
    imageView = createImageView(device,
                                image,
                                format,
                                1,
                                VK_IMAGE_VIEW_TYPE_CUBE,
                                6);
    if (imageView == VK_NULL_HANDLE)
        return;

    // Create sampler.
    sampler = createSampler(device,
                            magFilter,
                            minFilter,
                            addressModeU,
                            addressModeV,
                            addressModeW,
                            1);
    if (sampler == VK_NULL_HANDLE)
        return;

    // Define texture cube image regions in memory
    std::vector<VkBufferImageCopy> regions;
    offset = 0;
    for (uint32_t layer = 0; layer < 6; layer++)
    {
        VkBufferImageCopy region;
        region.bufferOffset      = offset;
        region.bufferRowLength   = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = layer;
        region.imageSubresource.layerCount     = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = extent;

        regions.push_back(region);

        offset += images[layer].byteCount();
    }

    // Allocate buffers for queue commands.
    VkCommandBuffer cmdBuf =
        commandPool.allocateBuffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Start recording commands
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    // Transition image into transfer destination layout
    commandTransitionImageLayout(
        image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        0, 6,
        cmdBuf);

    // Copy buffer to image.
    commandCopyBufferToImage(buf.handle(), image, cmdBuf, regions);

    // Transition image into optimal shader read layout.
    commandTransitionImageLayout(
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 6, cmdBuf);

    // Stop recording commands.
    const VkResult result = vkEndCommandBuffer(cmdBuf);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to apply texture cube commands as "
                  << vk::stringify::result(result)
                  << std::endl;
        return;
    }

    // Submit commands into queue
    if (!queue.submit(cmdBuf))
        return;

    // Wait until commands has been processed.
    if (!queue.waitIdle())
        return;
}

TextureCube::TextureCube(const VkPhysicalDevice& physicalDevice,
                         const VkDevice& device,
                         const VkExtent3D& extent,
                         const VkFormat& format,
                         VkFilter magFilter,
                         VkFilter minFilter,
                         VkSamplerAddressMode addressModeU,
                         VkSamplerAddressMode addressModeV,
                         VkSamplerAddressMode addressModeW,
                         bool mipmaps)
    : format(format)
    , image(VK_NULL_HANDLE)
    , imageView(VK_NULL_HANDLE)
    , sampler(VK_NULL_HANDLE)
    , impl(std::make_shared<Impl>(device, this))
{
    mipmapCount = 1;
    if (mipmaps)
        mipmapCount = uint32_t((floor(log2(extent.width))) + 1);

    // Crate texture cube image
    image = createImage(device,
                        format,
                        extent,
                        mipmapCount,
                        6,
                        VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    // Allocate memory.
    memory = allocateMemory(physicalDevice, device, image);
    if (memory == VK_NULL_HANDLE)
        return;

    // Create view.
    imageView = createImageView(device,
                                image,
                                format,
                                mipmapCount,
                                VK_IMAGE_VIEW_TYPE_CUBE,
                                6);
    if (imageView == VK_NULL_HANDLE)
        return;

    // Create sampler.
    sampler = createSampler(device,
                            magFilter,
                            minFilter,
                            addressModeU,
                            addressModeV,
                            addressModeW,
                            mipmapCount);
    if (sampler == VK_NULL_HANDLE)
        return;
}

bool transitionTexture(Queue& queue,
                       CommandPool& commandPool,
                       std::shared_ptr<Texture2D> texture,
                       VkImageLayout from,
                       VkImageLayout to)
{

    // Allocate buffers for queue commands.
    VkCommandBuffer cmdBuf =
        commandPool.allocateBuffer(
            VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    // Start recording commands
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdBuf, &beginInfo);

    // Transition image into optimal shader read layout.
    commandTransitionImageLayout(
        texture->image,
        from,
        to,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 6, cmdBuf);

    // Stop recording commands.
    const VkResult result = vkEndCommandBuffer(cmdBuf);
    if (result != VK_SUCCESS)
    {
        std::cerr << __FUNCTION__
                  << ": failed to transition texture as "
                  << vk::stringify::result(result)
                  << std::endl;
        return false;
    }

    // Submit commands into queue
    if (!queue.submit(cmdBuf))
        return false;

    // Wait until commands has been processed.
    return queue.waitIdle();
}

//bool transitionTexture(const VkCommandBuffer& cmdBuf,
//                       const VkImage& image,
//                       VkImageLayout from,
//                       VkImageLayout to)
//{
//    commandTransitionImageLayout(
//        image,
//        from,
//        to,
//        VK_PIPELINE_STAGE_TRANSFER_BIT,
//        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//        0, 6, cmdBuf);
//}

} // namespace vk
} // namespace kuu

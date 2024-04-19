#include <opencv2/opencv.hpp>

#include "Texture.h"

namespace AE {

    Texture::~Texture() {
        vkDestroyImage(m_devices.getLogicalDevice(), m_image, nullptr);
        vkFreeMemory(m_devices.getLogicalDevice(), m_imageMemory, nullptr);
        vkDestroyImageView(m_devices.getLogicalDevice(), m_imageView, nullptr);
        vkDestroySampler(m_devices.getLogicalDevice(), m_sampler, nullptr);
    }

    void Texture::createTextureImage(const char* filePath) {
        cv::Mat img = cv::imread(filePath, cv::IMREAD_UNCHANGED);
        if (img.data == NULL) {
            printf("file read error");
        }
        cv::cvtColor(img, img, cv::COLOR_BGR2RGBA);
        m_texWidth = img.cols;
        m_texHeight = img.rows;
        m_texChannels = img.channels();
        VkDeviceSize bytes_per_pixel = img.elemSize();
#ifdef ENABLE_MIPMAP
        m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_texWidth, m_texHeight)))) + 1;
#endif

        Buffer stagingBuffer{
            m_devices,
            bytes_per_pixel,
            static_cast<uint32_t>(m_texWidth * m_texHeight),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };

        // create a region of host(cpu) memory mapped to device memory and set data to point to the beginning of the mapped memory range
        stagingBuffer.map();
        // As we specified VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data will automatically be flushed to update device memory after memcpy
        stagingBuffer.writeToBuffer(img.data);

        if (m_texChannels == 3) {
            m_imageFormat = VK_FORMAT_R8G8B8_SRGB;
        }
        else if (m_texChannels == 4) {
            m_imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        }

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_imageFormat;
#ifdef ENABLE_MIPMAP
        imageInfo.mipLevels = m_mipLevels;
#else
        imageInfo.mipLevels = 1;
#endif
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // relevant for images that will be used as attachments
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // layout initialization. The value will be the first argument in transitionImageLayout(). 
        // This cannot be initialized with VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL. 
        // So, initialize with UNDEFINED at first. Then, change it into VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.extent = { static_cast<uint32_t>(m_texWidth), static_cast<uint32_t>(m_texHeight), 1 }; // {width, height, depth}

        // transfer from staging buffer and use this image for texture sampling
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

        // Allocate memory
        m_devices.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);
        // change layout from UNDEFINED to TRANSFER
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        // copy from buffer to image
        m_devices.copyBufferToImage(
            stagingBuffer.getBuffer(),
            m_image,
            static_cast<uint>(m_texWidth),
            static_cast<uint>(m_texHeight),
            1
        );
#ifdef ENABLE_MIPMAP
        generateMipmaps();
#else
        // change layout from TRANSFER to SHADER_READ_ONLY
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
#endif
        // generateMipmaps();
        m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    void Texture::createTextureImageView() {
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format = m_imageFormat;
        imageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
        imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewInfo.subresourceRange.baseMipLevel = 0;
        imageViewInfo.subresourceRange.baseArrayLayer = 0;
        imageViewInfo.subresourceRange.layerCount = 1;
#ifdef MIPMAP
        imageViewInfo.subresourceRange.levelCount = mipLevels;
#else
        imageViewInfo.subresourceRange.levelCount = 1;
#endif
        imageViewInfo.image = m_image;

        if(vkCreateImageView(m_devices.getLogicalDevice(), &imageViewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void Texture::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR; // Filter used when the object is close to the camera
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

        // behavior when going beyond the image dimensions.
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; // VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; // behavior when going beyond the image dimensions.
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT; // behavior when going beyond the image dimensions.
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.minLod = 0.f;
#ifdef ENABLE_MIPMAP
        samplerInfo.maxLod = static_cast<float>(m_mipLevels);
#else
        samplerInfo.maxLod = 0.f;
#endif
        samplerInfo.maxAnisotropy = 4.0;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        vkCreateSampler(m_devices.getLogicalDevice(), &samplerInfo, nullptr, &m_sampler);
    }

    void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
        VkCommandBuffer commandBuffer = m_devices.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
#ifdef ENABLE_MIPMAP
        barrier.subresourceRange.levelCount = m_mipLevels;
#else
        barrier.subresourceRange.levelCount = 1;
#endif
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::runtime_error("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer, 
            sourceStage, 
            destinationStage, 
            0, 
            0, 
            nullptr, 
            0, 
            nullptr, 
            1, 
            &barrier
        );

        m_devices.endSingleTimeCommands(commandBuffer);
    }

#ifdef ENABLE_MIPMAP
    void Texture::generateMipmaps() {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_devices.getPhysicalDevice(), m_imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer commandBuffer = m_devices.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = m_image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = m_texWidth;
        int32_t mipHeight = m_texHeight;

        for (uint32_t i = 1; i < m_mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer, 
                VK_PIPELINE_STAGE_TRANSFER_BIT, 
                VK_PIPELINE_STAGE_TRANSFER_BIT, 
                0, 
                0, 
                nullptr, 
                0, 
                nullptr, 
                1, 
                &barrier
            );

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(
                commandBuffer, 
                m_image, 
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
                m_image, 
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                1, 
                &blit, 
                VK_FILTER_LINEAR
            );

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                commandBuffer, 
                VK_PIPELINE_STAGE_TRANSFER_BIT, 
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
                0, 
                0, 
                nullptr, 
                0, 
                nullptr, 
                1, 
                &barrier
            );

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer, 
            VK_PIPELINE_STAGE_TRANSFER_BIT, 
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 
            0, 
            0, 
            nullptr, 
            0, 
            nullptr, 
            1, 
            &barrier
        );

        m_devices.endSingleTimeCommands(commandBuffer);
    }
#endif

} // namespace AE
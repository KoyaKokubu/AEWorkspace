#pragma once

#include <string.h>

#include "Utils/AREngineIncludes.h"
#include "Utils/AREngineDefines.h"
#include "Devices.h"
#include "Buffer.h"

namespace AE {

    class Texture {
    public:
        Texture(Devices& devices) : m_devices(devices) {};
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(Texture&&) = delete;

        void createTextureImage(const char* filePath);
        void generateMipmaps();
        void createTextureImageView();
        void createTextureSampler();

        VkSampler getSampler() { return m_sampler; }
        VkImageView getImageView() { return m_imageView; }
        VkImageLayout getImageLayout() { return m_imageLayout; }

    private:
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        int m_texWidth=0, m_texHeight=0, m_texChannels=0;
#ifdef ENABLE_MIPMAP
        int m_mipLevels;
#endif
        Devices& m_devices;
        VkImage m_image;
        VkDeviceMemory m_imageMemory;
        VkImageView m_imageView;
        VkSampler m_sampler;
        VkFormat m_imageFormat;
        VkImageLayout m_imageLayout;
    };

}
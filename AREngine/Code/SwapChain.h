#pragma once

#include "Utils/AREngineIncludes.h"

namespace AE {

	class Devices;
	class WinApplication;

	class SwapChain {
	public:
		SwapChain(AE::Devices& devices) : m_devices{ devices } {}

		// Not copyable or movable
		SwapChain(const SwapChain&) = delete;
		void operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;
		
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		void createSwapChain(AE::WinApplication& winApp);
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFrameBuffers();
		void createSyncObjects();

		VkFormat findDepthFormat();
		uint32_t width() { return m_swapChainExtent.width; }
		uint32_t height() { return m_swapChainExtent.height; }

		VkSwapchainKHR& getSwapChain() { return m_swapChain; }
		VkExtent2D& getSwapChainExtent() { return m_swapChainExtent; }
		std::vector<VkImageView>& getImageViews() { return m_swapChainImageViews;  }
		std::vector<VkImage>& getDepthImages() { return m_depthImages; }
		std::vector<VkImageView>& getDepthImageViews() { return m_depthImageViews; }
		std::vector<VkDeviceMemory>& getDepthImageMemorys() { return m_depthImageMemorys; }
		std::vector<VkFramebuffer>& getFrameBuffers() { return m_framebuffers; }
		VkRenderPass& getRenderPass() { return m_renderPass; }
		std::vector<VkSemaphore>& getImageAvailableSemaphores() { return m_imageAvailableSemaphores; }
		std::vector<VkSemaphore>& getRenderFinishedSemaphores() { return m_renderFinishedSemaphores; }
		std::vector<VkFence>& getInFlightFences() { return m_inFlightFences; }
		std::vector<VkFence>& getImagesInFlight() { return m_imagesInFlight; }
		size_t imageCount() { return m_swapChainImages.size(); }

	private:
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, AE::WinApplication& winApp);

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;

		Devices& m_devices;
		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImageView> m_swapChainImageViews; // can be used as color targets
		std::vector<VkImage> m_depthImages;
		std::vector<VkImageView> m_depthImageViews;
		std::vector<VkDeviceMemory> m_depthImageMemorys;
		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_framebuffers;
	};

} // namespace AE
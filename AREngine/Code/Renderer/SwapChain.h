#pragma once

#include <memory>

#include "../Utils/AREngineIncludes.h"

namespace AE {

	class Devices;
	class WinApplication;

	class SwapChain {
	public:
		SwapChain(Devices& devices) : m_devices{ devices } {}
		SwapChain(Devices& devices, std::shared_ptr<SwapChain> previous);

		// Not copyable or movable
		SwapChain(const SwapChain&) = delete;
		SwapChain& operator=(const SwapChain&) = delete;
		SwapChain(SwapChain&&) = delete;
		SwapChain& operator=(SwapChain&&) = delete;
		
		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		void createSwapChain(AE::WinApplication& winApp);
		void createImageViews();
		void createColorResources();
		void createDepthResources();
		void createRenderPass();
		void createFrameBuffers();
		void createSyncObjects();

		VkFormat findDepthFormat();
		bool compareSwapFormats(const SwapChain& swapChain) const {
			return swapChain.m_swapChainDepthFormat == m_swapChainDepthFormat
				&& swapChain.m_swapChainImageFormat == m_swapChainImageFormat;
		}
		float extentAspectRatio() {
			return static_cast<float>(m_swapChainExtent.width) / static_cast<float>(m_swapChainExtent.height);
		}

		uint32_t width() { return m_swapChainExtent.width; }
		uint32_t height() { return m_swapChainExtent.height; }
		const VkSwapchainKHR& getSwapChain() const { return m_swapChain; }
		const VkExtent2D& getSwapChainExtent() const { return m_swapChainExtent; }
		const std::vector<VkImageView>& getImageViews() const { return m_swapChainImageViews;  }
		const std::vector<VkImage>& getDepthImages() const { return m_depthImages; }
		const std::vector<VkImageView>& getDepthImageViews() const { return m_depthImageViews; }
		const std::vector<VkDeviceMemory>& getDepthImageMemorys() const { return m_depthImageMemorys; }
		const std::vector<VkImage>& getMSAAImages() const { return m_msaaImages; }
		const std::vector<VkImageView>& getMSAAImageViews() const { return m_msaaImageViews; }
		const std::vector<VkDeviceMemory>& getMSAAImageMemorys() const { return m_msaaImageMemorys; }
		const std::vector<VkFramebuffer>& getFrameBuffers() const { return m_framebuffers; }
		const VkRenderPass& getRenderPass() const { return m_renderPass; }
		const std::vector<VkSemaphore>& getImageAvailableSemaphores() const { return m_imageAvailableSemaphores; }
		const std::vector<VkSemaphore>& getRenderFinishedSemaphores() const { return m_renderFinishedSemaphores; }
		const std::vector<VkFence>& getInFlightFences() const { return m_inFlightFences; }
		const std::vector<VkFence>& getImagesInFlight() const { return m_imagesInFlight; }
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
		std::shared_ptr<SwapChain> m_oldSwapChain;
		std::vector<VkImage> m_swapChainImages;
		VkFormat m_swapChainImageFormat;
		VkFormat m_swapChainDepthFormat;
		VkExtent2D m_swapChainExtent;
		std::vector<VkImageView> m_swapChainImageViews; // can be used as color targets
		std::vector<VkImage> m_depthImages;
		std::vector<VkImageView> m_depthImageViews;
		std::vector<VkDeviceMemory> m_depthImageMemorys;
		std::vector<VkImage> m_msaaImages;
		std::vector<VkImageView> m_msaaImageViews;
		std::vector<VkDeviceMemory> m_msaaImageMemorys;
		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_framebuffers;
	};

} // namespace AE
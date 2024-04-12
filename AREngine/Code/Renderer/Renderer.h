#pragma once

#include <memory>
#include <cassert>

#include "../Utils/AREngineDefines.h"

#include "../Devices.h"
#include "WinApplication.h"
#include "SwapChain.h"

namespace AE {

	class Renderer {
	public:
		Renderer(WinApplication& window, Devices& devices) : m_winApp{ window }, m_devices{ devices } {}

		// Not copyable or movable
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		bool isFrameInProgress() const { return m_isFrameStarted; }
		VkRenderPass getSwapChainRenderPass() const { return m_swapChain->getRenderPass(); }
		float getAspectRatio() const { return m_swapChain->extentAspectRatio(); }
		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_commandBuffers[m_currentFrameIndex];
		}
		int getFrameIndex() const {
			assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
			return m_currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void recreateSwapChain();
		void cleanupSwapChain();
		void createCommandBuffers();

	private:
		void recordCommandBuffer(int imageIndex);
		void freeCommandBuffers();

		WinApplication& m_winApp;
		Devices& m_devices;
		std::unique_ptr<SwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex;
		bool m_isFrameStarted{ false };
	};

} // namespace AE
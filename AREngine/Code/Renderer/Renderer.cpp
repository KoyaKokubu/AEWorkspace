#include <cassert>
#include <map>
#include <array>

#include "Renderer.h"

namespace AE {

	void Renderer::createCommandBuffers() {
		// Each command buffer is going to draw to a different frame buffer.
		m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_devices.getCommandPool();
		// VK_COMMAND_BUFFER_LEVEL_PRIMARY : Can be submitted to a queue for execution, but cannot be called from other command buffers.
		// VK_COMMAND_BUFFER_LEVEL_SECONDARY : Cannot be submitted directly, but can be called from primary command buffers.
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_devices.getLogicalDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}

	void Renderer::freeCommandBuffers() {
		vkFreeCommandBuffers(
			m_devices.getLogicalDevice(),
			m_devices.getCommandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data()
		);
		m_commandBuffers.clear();
	}

	void Renderer::recreateSwapChain() {
		VkExtent2D extent = m_winApp.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = m_winApp.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_devices.getLogicalDevice());

		if (m_swapChain == nullptr) {
			m_swapChain = std::make_unique<SwapChain>(m_devices);
			m_swapChain->createSwapChain(m_winApp);
			m_swapChain->createImageViews();
#ifdef ENABLE_MSAA
			m_swapChain->createColorResources();
#endif
			m_swapChain->createDepthResources();
			m_swapChain->createRenderPass();
			m_swapChain->createFrameBuffers();
			m_swapChain->createSyncObjects();
		}
		else {
			cleanupSwapChain();
			std::shared_ptr<SwapChain> oldSwapChain = std::move(m_swapChain);
			m_swapChain = std::make_unique<SwapChain>(m_devices, oldSwapChain);
			m_swapChain->createSwapChain(m_winApp);
			m_swapChain->createImageViews();
#ifdef ENABLE_MSAA
			m_swapChain->createColorResources();
#endif
			m_swapChain->createDepthResources();
			m_swapChain->createRenderPass();
			m_swapChain->createFrameBuffers();
			m_swapChain->createSyncObjects();
			// check the compatibility between the current swapchain and the previous swapchain
			if (!oldSwapChain->compareSwapFormats(*m_swapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void Renderer::cleanupSwapChain() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_devices.getLogicalDevice(), m_swapChain->getImageAvailableSemaphores()[i], nullptr);
			vkDestroySemaphore(m_devices.getLogicalDevice(), m_swapChain->getRenderFinishedSemaphores()[i], nullptr);
			vkDestroyFence(m_devices.getLogicalDevice(), m_swapChain->getInFlightFences()[i], nullptr);
		}
		for (auto framebuffer : m_swapChain->getFrameBuffers()) {
			vkDestroyFramebuffer(m_devices.getLogicalDevice(), framebuffer, nullptr);
		}
		vkDestroyRenderPass(m_devices.getLogicalDevice(), m_swapChain->getRenderPass(), nullptr);
		for (int i = 0; i < m_swapChain->getDepthImages().size(); i++) {
			vkDestroyImageView(m_devices.getLogicalDevice(), m_swapChain->getDepthImageViews()[i], nullptr);
			vkDestroyImage(m_devices.getLogicalDevice(), m_swapChain->getDepthImages()[i], nullptr);
			vkFreeMemory(m_devices.getLogicalDevice(), m_swapChain->getDepthImageMemorys()[i], nullptr);
		}
#ifdef ENABLE_MSAA
		for (int i = 0; i < m_swapChain->getDepthImages().size(); i++) {
			vkDestroyImageView(m_devices.getLogicalDevice(), m_swapChain->getMSAAImageViews()[i], nullptr);
			vkDestroyImage(m_devices.getLogicalDevice(), m_swapChain->getMSAAImages()[i], nullptr);
			vkFreeMemory(m_devices.getLogicalDevice(), m_swapChain->getMSAAImageMemorys()[i], nullptr);
		}
#endif
		for (VkImageView imageView : m_swapChain->getImageViews()) {
			vkDestroyImageView(m_devices.getLogicalDevice(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_devices.getLogicalDevice(), m_swapChain->getSwapChain(), nullptr);
	}

	VkCommandBuffer Renderer::beginFrame() {
		assert(!m_isFrameStarted && "Can't call beginFrame while already in progress");

		VkResult result = m_swapChain->acquireNextImage(&m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		m_isFrameStarted = true;
		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		return commandBuffer;
	}

	void Renderer::endFrame() {
		assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress");
		VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		VkResult result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
		// VK_SUBOPTIMAL_KHR : swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_winApp.wasWindowResized()) {
			m_winApp.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to record command buffer!");
		}

		m_isFrameStarted = false;
		m_currentFrameIndex = (m_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame"
		);
		
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapChain->getRenderPass();
		renderPassInfo.framebuffer = m_swapChain->getFrameBuffers()[m_currentImageIndex];
		// renderArea : where shader loads and stores will take place. The pixels outside this region will have undefined values.
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f }; // index 0 : layout 0 as defined in render pass -> color buffer
		clearValues[1].depthStencil = { 1.0f, 0 }; // index 1 : layout 1 as defined in render pass -> depth stencil buffer
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		// All of the functions that record commands can be recognized by their vkCmd prefix.
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_swapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(m_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame"
		);
		vkCmdEndRenderPass(commandBuffer);
	}

} // namespace AE
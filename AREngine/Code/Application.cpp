#include <map>
#include <array>

#include "Application.h"

namespace AE {

	void Application::run() {
		initVulkan();
		mainLoop();
		cleanup();
	}

	void Application::initVulkan() {
		m_winApp.initWindow();
		m_vkInstance.createInstance();
		m_validLayers.setupDebugMessenger(m_vkInstance.getInstance());
		m_devices.createSurface(m_vkInstance.getInstance(), m_winApp);
		m_devices.pickPhysicalDevice(m_vkInstance.getInstance());
		m_devices.createLogicalDevice();
		m_swapChain.createSwapChain(m_winApp);
		m_swapChain.createImageViews();
		m_swapChain.createDepthResources();
		m_swapChain.createRenderPass();
		createPipelineLayout();
		createGraphicsPipeline();
		m_swapChain.createFrameBuffers();
		m_devices.createCommandPool();
		createCommandBuffers();
		recordCommandBuffers();
		m_swapChain.createSyncObjects();
	}

	void Application::mainLoop() {
		while (!m_winApp.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(m_devices.getLogicalDevice());
	}

	void Application::cleanup() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_devices.getLogicalDevice(), m_swapChain.getImageAvailableSemaphores()[i], nullptr);
			vkDestroySemaphore(m_devices.getLogicalDevice(), m_swapChain.getRenderFinishedSemaphores()[i], nullptr);
			vkDestroyFence(m_devices.getLogicalDevice(), m_swapChain.getInFlightFences()[i], nullptr);
		}
		vkDestroyCommandPool(m_devices.getLogicalDevice(), m_devices.getCommandPool(), nullptr);
		for (auto framebuffer : m_swapChain.getFrameBuffers()) {
			vkDestroyFramebuffer(m_devices.getLogicalDevice(), framebuffer, nullptr);
		}
		vkDestroyPipeline(m_devices.getLogicalDevice(), m_GraphicsPipeline->getGraphicsPipeline(), nullptr);
		vkDestroyPipelineLayout(m_devices.getLogicalDevice(), m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_devices.getLogicalDevice(), m_swapChain.getRenderPass(), nullptr);
		for (int i = 0; i < m_swapChain.getDepthImages().size(); i++) {
			vkDestroyImageView(m_devices.getLogicalDevice(), m_swapChain.getDepthImageViews()[i], nullptr);
			vkDestroyImage(m_devices.getLogicalDevice(), m_swapChain.getDepthImages()[i], nullptr);
			vkFreeMemory(m_devices.getLogicalDevice(), m_swapChain.getDepthImageMemorys()[i], nullptr);
		}
		for (VkImageView imageView : m_swapChain.getImageViews()) {
			vkDestroyImageView(m_devices.getLogicalDevice(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_devices.getLogicalDevice(), m_swapChain.getSwapChain(), nullptr);
		vkDestroyDevice(m_devices.getLogicalDevice(), nullptr);
		if (m_validLayers.enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_vkInstance.getInstance(), m_validLayers.m_debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(m_vkInstance.getInstance(), m_devices.getSurface(), nullptr);
		vkDestroyInstance(m_vkInstance.getInstance(), nullptr);
		glfwDestroyWindow(m_winApp.getWindowPointer());
		glfwTerminate();
	}

	// "uniform" values in shaders, which are globals similar to dynamic state variables that can be changed at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to pass the 
	// (ex) transformation matrix, texture samplers
	void Application::createPipelineLayout() {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(m_devices.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void Application::createGraphicsPipeline() {
		GraphicsPipeline::defaultPipelineConfig(m_swapChain.width(), m_swapChain.height(), m_pipelineConfig);
		m_pipelineConfig.renderPass = m_swapChain.getRenderPass();
		m_pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(m_devices, SYSTEM_FILE_PATH);
		m_GraphicsPipeline->createGraphicsPipeline(VERT_SHADER_PATH, FRAG_SHADER_PATH, m_pipelineConfig);
	}

	void Application::createCommandBuffers() {
		// Each command buffer is going to draw to a different frame buffer.
		m_commandBuffers.resize(m_swapChain.imageCount()); // 2 or 3 swap chain images according to double or triple buffering

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

	void Application::recordCommandBuffers() {
		for (int i = 0; i < m_commandBuffers.size(); i++) {
			//vkResetCommandBuffer(m_commandBuffers[i], 0);
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = 0; // Optional
			beginInfo.pInheritanceInfo = nullptr; // Optional
			if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_swapChain.getRenderPass();
			renderPassInfo.framebuffer = m_swapChain.getFrameBuffers()[i];
			// renderArea : where shader loads and stores will take place. The pixels outside this region will have undefined values.
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_swapChain.getSwapChainExtent();
			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // index 0 : layout 0 as defined in render pass -> color buffer
			clearValues[1].depthStencil = { 1.0f, 0 }; // index 1 : layout 1 as defined in render pass -> depth stencil buffer
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();
			// All of the functions that record commands can be recognized by their vkCmd prefix.
			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(m_commandBuffers[i], 0, 1, &m_pipelineConfig.viewport);
			vkCmdSetScissor(m_commandBuffers[i], 0, 1, &m_pipelineConfig.scissor);

			m_GraphicsPipeline->bind(m_commandBuffers[i]);

			// vkCmdDraw(m_commandBuffers[i], vertexCount, instanceCount, firstVertex, firstInstance);
			// vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
			// instanceCount: Used for instanced rendering, use 1 if you're not doing that.
			// firstVertex : Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
			// firstInstance : Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
			vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(m_commandBuffers[i]);
			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}
	}

	// all of the operations in drawFrame are asynchronous. 
	void Application::drawFrame() {
		uint32_t imageIndex;
		VkResult result = m_swapChain.acquireNextImage(&imageIndex); // Sysn with Fence, then acquire image from swap chain
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to record command buffer!");
		}
		
		// Draw a frame and display it
		result = m_swapChain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

} // namespace AE
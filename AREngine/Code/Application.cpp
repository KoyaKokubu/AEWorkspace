#include <cassert>
#include <map>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Application.h"

namespace AE {

	// must match the order specified in shaders
	struct SimplePushConstantData {
		glm::vec2 offset;
		alignas(16) glm::vec3 color; // GPU memory alignment requirement
	};

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
		createPipelineLayout();
		recreateSwapChain();
		m_devices.createCommandPool();
		loadModels();
		createCommandBuffers();
	}

	void Application::mainLoop() {
		while (!m_winApp.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}
		vkDeviceWaitIdle(m_devices.getLogicalDevice());
	}

	void Application::cleanup() {
		cleanupSwapChain();
		vkDestroyCommandPool(m_devices.getLogicalDevice(), m_devices.getCommandPool(), nullptr);
		vkDestroyPipelineLayout(m_devices.getLogicalDevice(), m_pipelineLayout, nullptr);
		vkDestroyBuffer(m_devices.getLogicalDevice(), m_model->getVertexBuffer(), nullptr);
		vkFreeMemory(m_devices.getLogicalDevice(), m_model->getVertexBufferMemory(), nullptr);
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
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // 
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_devices.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void Application::createGraphicsPipeline() {
		assert(m_swapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		GraphicsPipeline::defaultPipelineConfig(pipelineConfig);
		pipelineConfig.renderPass = m_swapChain->getRenderPass();
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_GraphicsPipeline = std::make_unique<GraphicsPipeline>(m_devices, SYSTEM_FILE_PATH);
		m_GraphicsPipeline->createGraphicsPipeline(VERT_SHADER_PATH, FRAG_SHADER_PATH, pipelineConfig);
	}

	void Application::createCommandBuffers() {
		// Each command buffer is going to draw to a different frame buffer.
		m_commandBuffers.resize(m_swapChain->imageCount()); // 2 or 3 swap chain images according to double or triple buffering

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

	void Application::recordCommandBuffer(int imageIndex) {
		static int frame = 30;
		frame = (frame + 1) % 100;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional
		if (vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapChain->getRenderPass();
		renderPassInfo.framebuffer = m_swapChain->getFrameBuffers()[imageIndex];
		// renderArea : where shader loads and stores will take place. The pixels outside this region will have undefined values.
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; // index 0 : layout 0 as defined in render pass -> color buffer
		clearValues[1].depthStencil = { 1.0f, 0 }; // index 1 : layout 1 as defined in render pass -> depth stencil buffer
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();
		// All of the functions that record commands can be recognized by their vkCmd prefix.
		vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(m_swapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, m_swapChain->getSwapChainExtent() };
		vkCmdSetViewport(m_commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(m_commandBuffers[imageIndex], 0, 1, &scissor);

		m_GraphicsPipeline->bind(m_commandBuffers[imageIndex]);
		m_model->bind(m_commandBuffers[imageIndex]);

		for (int j = 0; j < 4; j++) {
			SimplePushConstantData push{};
			push.offset = { -0.5f + frame * 0.02f, -0.4f + j * 0.25f };
			push.color = { 0.0f, 0.0f, 0.2f + 0.2f * j };

			vkCmdPushConstants(
				m_commandBuffers[imageIndex],
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);
			m_model->draw(m_commandBuffers[imageIndex]); // write a draw call in the command buuffer
		}

		vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(m_commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void Application::freeCommandBuffers() {
		vkFreeCommandBuffers(
			m_devices.getLogicalDevice(),
			m_devices.getCommandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data()
		);
		m_commandBuffers.clear();
	}

	void Application::recreateSwapChain() {
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
			m_swapChain->createDepthResources();
			m_swapChain->createRenderPass();
			m_swapChain->createFrameBuffers();
			m_swapChain->createSyncObjects();
		}
		else {
			cleanupSwapChain();
			m_swapChain = std::make_unique<SwapChain>(m_devices, std::move(m_swapChain));
			m_swapChain->createSwapChain(m_winApp);
			m_swapChain->createImageViews();
			m_swapChain->createDepthResources();
			m_swapChain->createRenderPass();
			m_swapChain->createFrameBuffers();
			m_swapChain->createSyncObjects();
			if (m_swapChain->imageCount() != m_commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}

		// Possible optimization here is to check the compatibility of the current render pass and the previous one.
		// If the render pass is compatible, then not need to create a pipeline.
		createGraphicsPipeline();
	}
	
	void Application::cleanupSwapChain() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_devices.getLogicalDevice(), m_swapChain->getImageAvailableSemaphores()[i], nullptr);
			vkDestroySemaphore(m_devices.getLogicalDevice(), m_swapChain->getRenderFinishedSemaphores()[i], nullptr);
			vkDestroyFence(m_devices.getLogicalDevice(), m_swapChain->getInFlightFences()[i], nullptr);
		}
		for (auto framebuffer : m_swapChain->getFrameBuffers()) {
			vkDestroyFramebuffer(m_devices.getLogicalDevice(), framebuffer, nullptr);
		}
		vkDestroyPipeline(m_devices.getLogicalDevice(), m_GraphicsPipeline->getGraphicsPipeline(), nullptr);
		vkDestroyRenderPass(m_devices.getLogicalDevice(), m_swapChain->getRenderPass(), nullptr);
		for (int i = 0; i < m_swapChain->getDepthImages().size(); i++) {
			vkDestroyImageView(m_devices.getLogicalDevice(), m_swapChain->getDepthImageViews()[i], nullptr);
			vkDestroyImage(m_devices.getLogicalDevice(), m_swapChain->getDepthImages()[i], nullptr);
			vkFreeMemory(m_devices.getLogicalDevice(), m_swapChain->getDepthImageMemorys()[i], nullptr);
		}
		for (VkImageView imageView : m_swapChain->getImageViews()) {
			vkDestroyImageView(m_devices.getLogicalDevice(), imageView, nullptr);
		}
		vkDestroySwapchainKHR(m_devices.getLogicalDevice(), m_swapChain->getSwapChain(), nullptr);
	}

	void Application::loadModels() {
		/*std::vector<Model::Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};*/
		std::vector<Model::Vertex> vertices = {
			{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		m_model = std::make_unique<Model>(m_devices);
		m_model->createVertexBuffers(vertices);
	}

	// all of the operations in drawFrame are asynchronous. 
	void Application::drawFrame() {
		uint32_t imageIndex;
		VkResult result = m_swapChain->acquireNextImage(&imageIndex); // Sysn with Fence, then acquire image from swap chain
		
		// VK_ERROR_OUT_OF_DATE_KHR : surface has changed in such a way that it is no longer compatible with the swapchain.
		// So, applications must query the new surface properties and recreate swapchain.
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to record command buffer!");
		}

		// Draw a frame and display it
		recordCommandBuffer(imageIndex);
		result = m_swapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);

		// VK_SUBOPTIMAL_KHR : swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully.
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_winApp.wasWindowResized()) {
			m_winApp.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

} // namespace AE
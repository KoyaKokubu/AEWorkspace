#include <cassert>
#include <map>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
		m_simpleRenderSystem.createPipelineLayout();
		m_renderer.recreateSwapChain();
		m_simpleRenderSystem.createGraphicsPipeline(m_renderer.getSwapChainRenderPass());
		m_devices.createCommandPool();
		loadGameObjects();
		m_renderer.createCommandBuffers();
	}

	void Application::mainLoop() {
		while (!m_winApp.shouldClose()) {
			glfwPollEvents();

			if (VkCommandBuffer commandBuffer = m_renderer.beginFrame()) {
				m_renderer.beginSwapChainRenderPass(commandBuffer);
				m_simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects);
				m_renderer.endSwapChainRenderPass(commandBuffer);
				m_renderer.endFrame();
			}
		}
		vkDeviceWaitIdle(m_devices.getLogicalDevice());
	}

	void Application::cleanup() {
		m_renderer.cleanupSwapChain();
		m_simpleRenderSystem.cleanupGraphicsPipeline();
		vkDestroyCommandPool(m_devices.getLogicalDevice(), m_devices.getCommandPool(), nullptr);
		for (GameObject& obj : m_gameObjects) {
			vkDestroyBuffer(m_devices.getLogicalDevice(), obj.m_model->getVertexBuffer(), nullptr);
			vkFreeMemory(m_devices.getLogicalDevice(), obj.m_model->getVertexBufferMemory(), nullptr);
			break;
		}
		vkDestroyDevice(m_devices.getLogicalDevice(), nullptr);
		if (m_validLayers.enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_vkInstance.getInstance(), m_validLayers.m_debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(m_vkInstance.getInstance(), m_devices.getSurface(), nullptr);
		vkDestroyInstance(m_vkInstance.getInstance(), nullptr);
		glfwDestroyWindow(m_winApp.getWindowPointer());
		glfwTerminate();
	}

	void Application::loadGameObjects() {
		std::vector<Model::Vertex> vertices{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}} 
		};
		std::shared_ptr<Model> model = std::make_shared<Model>(m_devices);
		model->createVertexBuffers(vertices);

		// https://www.color-hex.com/color-palette/5361
		std::vector<glm::vec3> colors{
			{1.f, .7f, .73f},
			{1.f, .87f, .73f},
			{1.f, 1.f, .73f},
			{.73f, 1.f, .8f},
			{.73, .88f, 1.f}
		};
		for (glm::vec3& color : colors) {
			color = glm::pow(color, glm::vec3{ 2.2f });
		}
		for (int i = 0; i < 40; i++) {
			GameObject triangle = GameObject::createGameObject();
			triangle.m_model = model;
			triangle.m_transform2dMat.scale = glm::vec2(.5f) + i * 0.025f;
			triangle.m_transform2dMat.rotation = i * glm::pi<float>() * .025f;
			triangle.m_color = colors[i % colors.size()];
			m_gameObjects.push_back(std::move(triangle));
		}
	}

} // namespace AE
#include <cassert>
#include <map>
#include <array>
#include <chrono> // current system time with high precision

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
		//m_camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
		m_camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f)); // second parameter : cube position

		GameObject viewerObject = GameObject::createGameObject();

		std::chrono::steady_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
		std::chrono::steady_clock::time_point prevTime = std::chrono::high_resolution_clock::now();

		while (!m_winApp.shouldClose()) {
			glfwPollEvents();

			std::chrono::steady_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - prevTime).count();
			prevTime = currentTime;

			m_cameraController.moveInPlaneXZ(m_winApp.getWindowPointer(), frameTime, viewerObject);
			m_camera.setViewYXZ(viewerObject.m_transformMat.m_translation, viewerObject.m_transformMat.m_rotation);
			float passTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - beginTime).count();
			/*if (int(passTime) % 10000 == 0) {
				printf("m_translation: %d, %d, %d\n", viewerObject.m_transformMat.m_translation.x, viewerObject.m_transformMat.m_translation.y, viewerObject.m_transformMat.m_translation.z);
			}*/

			float aspect = m_renderer.getAspectRatio();
			//m_camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			m_camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (VkCommandBuffer commandBuffer = m_renderer.beginFrame()) {
				m_renderer.beginSwapChainRenderPass(commandBuffer);
				m_simpleRenderSystem.renderGameObjects(commandBuffer, m_gameObjects, m_camera);
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
			if (obj.m_model->m_hasIndexBuffer) {
				vkDestroyBuffer(m_devices.getLogicalDevice(), obj.m_model->getIndexBuffer(), nullptr);
				vkFreeMemory(m_devices.getLogicalDevice(), obj.m_model->getIndexBufferMemory(), nullptr);
			}
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
		std::shared_ptr<Model> model = Model::createModelFromFile(m_devices, MODEL_DIRECTORY_PATH);
		GameObject gameObj = GameObject::createGameObject();
		gameObj.m_model = model;
		gameObj.m_transformMat.m_translation = { .0f, .0f, 2.5f };
		gameObj.m_transformMat.m_scale = glm::vec3(3.f);
		m_gameObjects.emplace_back(std::move(gameObj));
	}

} // namespace AE
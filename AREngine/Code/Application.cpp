#include <cassert>
#include <map>
#include <array>
#include <chrono> // current system time with high precision

#include "Application.h"
#include "Buffer.h"

namespace AE {

	// Global Uniform Buffer Object
	struct GlobalUBO {
		glm::mat4 projectionView{ 1.f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
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
		m_globalSetLayout =
			DescriptorSetLayout::Builder(m_devices)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
			.build();
		m_simpleRenderSystem.createPipelineLayout(m_globalSetLayout->getDescriptorSetLayout());
		m_renderer.recreateSwapChain();
		m_simpleRenderSystem.createGraphicsPipeline(m_renderer.getSwapChainRenderPass());
		m_devices.createCommandPool();
		m_globalPool = 
			DescriptorPool::Builder(m_devices)
			.setMaxSets(MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
			//.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
		m_renderer.createCommandBuffers();
	}

	void Application::mainLoop() {
		std::vector<std::unique_ptr<Buffer>> uboBuffers(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<Buffer>(
				m_devices,
				sizeof(GlobalUBO),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->map();
		}

		std::vector<VkDescriptorSet> globalDescriptorSets(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*m_globalSetLayout, *m_globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

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
			float passedTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - beginTime).count();

			float aspect = m_renderer.getAspectRatio();
			//m_camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
			m_camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

			if (VkCommandBuffer commandBuffer = m_renderer.beginFrame()) {
				int frameIndex = m_renderer.getFrameIndex();
				FrameInfo frameinfo{
					frameIndex,
					frameTime,
					commandBuffer,
					m_camera,
					globalDescriptorSets[frameIndex]
				};

				// update
				GlobalUBO ubo{};
				ubo.projectionView = m_camera.getProjection() * m_camera.getView();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				m_renderer.beginSwapChainRenderPass(commandBuffer);
				m_simpleRenderSystem.renderGameObjects(frameinfo, m_gameObjects);
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
		m_globalSetLayout = nullptr; // call destructor
		m_globalPool = nullptr; // call destructor
		m_gameObjects.clear(); // call destructor
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
		std::shared_ptr<Model> model = Model::createModelFromFile(m_devices, "Models/smooth_vase.obj");
		GameObject smooth_vase = GameObject::createGameObject();
		smooth_vase.m_model = model;
		smooth_vase.m_transformMat.m_translation = { -.5f, .5f, 2.5f };
		smooth_vase.m_transformMat.m_scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.emplace_back(std::move(smooth_vase));

		model = Model::createModelFromFile(m_devices, "Models/flat_vase.obj");
		GameObject flat_vase = GameObject::createGameObject();
		flat_vase.m_model = model;
		flat_vase.m_transformMat.m_translation = { .5f, .5f, 2.5f };
		flat_vase.m_transformMat.m_scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.emplace_back(std::move(flat_vase));
	}

} // namespace AE
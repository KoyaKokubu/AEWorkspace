#include <cassert>
#include <map>
#include <array>
#include <chrono> // current system time with high precision

#include "Application.h"
#include "Buffer.h"
#include "Texture.h"

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
		/*m_globalSetLayout =
			DescriptorSetLayout::Builder(m_devices)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();*/
		// global descriptor set layout
		m_descriptorSetLayouts.emplace_back(
			DescriptorSetLayout::Builder(m_devices)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build()
		);
		// texture descriptor set layout
		m_descriptorSetLayouts.emplace_back(
			DescriptorSetLayout::Builder(m_devices)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build()
		);
		for (int i = 0; i < m_descriptorSetLayouts.size(); i++) {
			m_VkDescriptorSetLayouts.emplace_back(m_descriptorSetLayouts[i]->getDescriptorSetLayout());
		}
		m_simpleRenderSystem.createPipelineLayout(m_VkDescriptorSetLayouts);
		m_pointLightSystem.createPipelineLayout(m_VkDescriptorSetLayouts[0]);
		/*m_simpleRenderSystem.createPipelineLayout(m_globalSetLayout->getDescriptorSetLayout());
		m_pointLightSystem.createPipelineLayout(m_globalSetLayout->getDescriptorSetLayout());*/
		m_renderer.recreateSwapChain();
		m_simpleRenderSystem.createGraphicsPipeline(m_renderer.getSwapChainRenderPass());
		m_pointLightSystem.createGraphicsPipeline(m_renderer.getSwapChainRenderPass());
		m_devices.createCommandPool();
		m_globalPool = 
			DescriptorPool::Builder(m_devices)
			.setMaxSets(MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_FRAMES_IN_FLIGHT)
			//.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
			//.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, MAX_FRAMES_IN_FLIGHT)
			.build();
		m_texturePool =
			DescriptorPool::Builder(m_devices)
			.setMaxSets(MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_FRAMES_IN_FLIGHT)
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

		/*VkDescriptorImageInfo imageInfo{};
		for (auto& kv : m_gameObjects) {
			GameObject& obj = kv.second;
			if (obj.m_model == nullptr || obj.m_model->m_texture == nullptr) {
				continue;
			}
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = obj.m_model->m_texture->getImageView();
			imageInfo.sampler = obj.m_model->m_texture->getSampler();
		}
		std::vector<VkDescriptorSet> globalDescriptorSets(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*m_globalSetLayout, *m_globalPool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.build(globalDescriptorSets[i]);
		}*/
		std::vector<std::vector<VkDescriptorSet>> descriptorSets(MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < descriptorSets.size(); i++) {
			descriptorSets[i].resize(descriptorSets.size());
			VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*m_descriptorSetLayouts[0], *m_globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(descriptorSets[i][0]);
		}

		VkDescriptorImageInfo imageInfo{};
		for (auto& kv : m_gameObjects) {
			GameObject& obj = kv.second;
			if (obj.m_model == nullptr || obj.m_model->m_texture == nullptr) {
				continue;
			}
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = obj.m_model->m_texture->getImageView();
			imageInfo.sampler = obj.m_model->m_texture->getSampler();
		}
		for (int i = 0; i < descriptorSets.size(); i++) {
			for (int j = 1; j < m_descriptorSetLayouts.size(); j++) {
				DescriptorWriter(*m_descriptorSetLayouts[j], *m_texturePool)
					.writeImage(0, &imageInfo)
					.build(descriptorSets[i][j]);
			}
		}

		//m_camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
		m_camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f)); // second parameter : cube position
		GameObject viewerObject = GameObject::createGameObject();
		viewerObject.m_transformMat.m_translation.z = -2.5f;

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
			m_camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

			if (VkCommandBuffer commandBuffer = m_renderer.beginFrame()) {
				int frameIndex = m_renderer.getFrameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					m_camera,
					descriptorSets[frameIndex],
					//globalDescriptorSets[frameIndex],
					m_gameObjects
				};

				// update
				GlobalUBO ubo{};
				ubo.projection = m_camera.getProjection();
				ubo.view = m_camera.getView();
				ubo.inverseView = m_camera.getInverseView();
				m_pointLightSystem.update(frameInfo, ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				m_renderer.beginSwapChainRenderPass(commandBuffer);

				// render solid objects first, then render any semi-transparent objects
				m_simpleRenderSystem.renderGameObjects(frameInfo);
				m_pointLightSystem.render(frameInfo);
				
				m_renderer.endSwapChainRenderPass(commandBuffer);
				m_renderer.endFrame();
			}
		}
		vkDeviceWaitIdle(m_devices.getLogicalDevice());
	}

	void Application::cleanup() {
		m_renderer.cleanupSwapChain();
		m_simpleRenderSystem.cleanupGraphicsPipeline();
		m_pointLightSystem.cleanupGraphicsPipeline();
		vkDestroyCommandPool(m_devices.getLogicalDevice(), m_devices.getCommandPool(), nullptr);
		//m_globalSetLayout = nullptr; // call destructor
		for (int i = 0; i < m_descriptorSetLayouts.size(); i++) {
			m_VkDescriptorSetLayouts[i] = nullptr;
			m_descriptorSetLayouts[i] = nullptr;
		}
		m_globalPool = nullptr; // call destructor
		m_texturePool = nullptr; // call destructor
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
		smooth_vase.m_transformMat.m_translation = { 0.f, .5f, 0.f };
		smooth_vase.m_transformMat.m_scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.emplace(smooth_vase.getId(), std::move(smooth_vase));

		/*model = Model::createModelFromFile(m_devices, "Models/flat_vase.obj");
		GameObject flat_vase = GameObject::createGameObject();
		flat_vase.m_model = model;
		flat_vase.m_transformMat.m_translation = { .5f, .5f, 0.f };
		flat_vase.m_transformMat.m_scale = { 3.f, 1.5f, 3.f };
		m_gameObjects.emplace(flat_vase.getId(), std::move(flat_vase));*/

		model = Model::createModelFromFile(m_devices, "Models/quad.obj");
		GameObject floor = GameObject::createGameObject();
		floor.m_model = model;
		floor.m_model->createTexture("Textures/ReadyPlayerMe-Avatar.jpeg");
		//floor.m_model->createTexture("Textures/meme.png");
		floor.m_transformMat.m_translation = { 0.f, .5f, 0.f };
		floor.m_transformMat.m_scale = { 3.f, 1.f, 3.f };
		m_gameObjects.emplace(floor.getId(), std::move(floor));

		std::vector<glm::vec3> lightColors{
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		for (int i = 0; i < lightColors.size(); i++) {
			GameObject pointLight = GameObject::makePointLight(0.2f);
			pointLight.m_color = lightColors[i];
			glm::highp_mat4 rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{0.f, -1.f, 0.f}
			);
			pointLight.m_transformMat.m_translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			m_gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}
	}

} // namespace AE
#pragma once

#include <iostream>
#include <optional>
#include <memory>

#include "Utils/AREngineDefines.h"

#include "VulkanInstance.h"
#include "Utils/ValidationLayers.h"
#include "Renderer/Renderer.h"
#include "RenderSystem/SimpleRenderSystem.h"
#include "RenderSystem/PointLightSystem.h"
#include "Camera.h"
#include "Input/KeyboardMovementController.h"
#include "Descriptors.h"

namespace AE {

	class Application {
	public:
		Application() {}

		// Not copyable or movable
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;
		
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		const char* m_appName = APPLICATION_NAME;

		void run();

	private:
		void initVulkan();
		void mainLoop();
		void cleanup();

		void loadGameObjects();

		WinApplication m_winApp{ WIDTH, HEIGHT, m_appName };
		ValidationLayers m_validLayers;
		VulkanInstance m_vkInstance{ m_appName, m_validLayers };
		Devices m_devices{ m_validLayers };
		Renderer m_renderer{ m_winApp, m_devices };
		SimpleRenderSystem m_simpleRenderSystem{ m_devices };
		PointLightSystem m_pointLightSystem{ m_devices };
		std::unique_ptr<DescriptorPool> m_globalPool{};
		std::unique_ptr<DescriptorPool> m_texturePool{};
		//std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
		std::vector<std::unique_ptr<DescriptorSetLayout>> m_descriptorSetLayouts;
		std::vector<VkDescriptorSetLayout> m_VkDescriptorSetLayouts;
		GameObject::Map m_gameObjects;
		Camera m_camera{};
		KeyboardMovementController m_cameraController{};
	};

} // namespace AE
#pragma once

#include <iostream>
#include <optional>
#include <memory>

#include "Utils/AREngineDefines.h"

#include "WinApplication.h"
#include "VulkanInstance.h"
#include "Utils/ValidationLayers.h"
#include "Devices.h"
#include "Renderer.h"
#include "SimpleRenderSystem.h"
#include "GameObject.h"

namespace AE {

	class Application {
	public:
		Application() {
		}

		// Not copyable or movable
		Application(const Application&) = delete;
		Application& operator=(const Application&) = delete;
		Application(Application&&) = delete;
		Application& operator=(Application&&) = delete;
		
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		const char* m_appName = "Vulkan Application";

		void run();

	private:
		void initVulkan();
		void mainLoop();
		void cleanup();

		void loadGameObjects();

		// systemFilePath : path to the system file which compiles shader files
		const char* m_systemFilePath = SYSTEM_FILE_PATH;
		const char* m_vertFilePath = VERT_SHADER_PATH;
		const char* m_fragFilePath = FRAG_SHADER_PATH;

		WinApplication m_winApp{ WIDTH, HEIGHT, m_appName };
		ValidationLayers m_validLayers;
		VulkanInstance m_vkInstance{ m_appName, m_validLayers };
		Devices m_devices{ m_validLayers };
		Renderer m_renderer{ m_winApp, m_devices };
		SimpleRenderSystem m_simpleRenderSystem{ m_devices };
		std::vector<GameObject> m_gameObjects;
	};

} // namespace AE
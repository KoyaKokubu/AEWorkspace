#pragma once

#include <iostream>
#include <optional>
#include <memory>

#include "Utils/AREngineDefines.h"

#include "WinApplication.h"
#include "VulkanInstance.h"
#include "Utils/ValidationLayers.h"
#include "Devices.h"
#include "SwapChain.h"

#include "GraphicsPipeline.h"

namespace AE {

	class Application {
	public:
		Application() {
		}

		// Not copyable or movable
		Application(const Application&) = delete;
		void operator=(const Application&) = delete;
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

		void createPipelineLayout();
		void createGraphicsPipeline();
		void createCommandBuffers();
		void recordCommandBuffers();
		void drawFrame();

		// systemFilePath : path to the system file which compiles shader files
		const char* m_systemFilePath = SYSTEM_FILE_PATH;
		const char* m_vertFilePath = VERT_SHADER_PATH;
		const char* m_fragFilePath = FRAG_SHADER_PATH;

		WinApplication m_winApp{ WIDTH, HEIGHT, m_appName };
		ValidationLayers m_validLayers;
		VulkanInstance m_vkInstance{ m_appName, m_validLayers };
		Devices m_devices{ m_validLayers };
		SwapChain m_swapChain{ m_devices };
		VkPipelineLayout m_pipelineLayout;
		PipelineConfigInfo m_pipelineConfig;
		std::unique_ptr<GraphicsPipeline> m_GraphicsPipeline;
		std::vector<VkCommandBuffer> m_commandBuffers;
	};

} // namespace AE
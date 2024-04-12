#pragma once

#include <iostream>
#include <optional>
#include <memory>

#include "Utils/AREngineDefines.h"

#include "Devices.h"
#include "GraphicsPipeline.h"
#include "GameObject.h"

namespace AE {

	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(Devices& devices) : m_devices{ devices } {}

		// Not copyable or movable
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem(SimpleRenderSystem&&) = delete;
		SimpleRenderSystem& operator=(SimpleRenderSystem&&) = delete;

		void createPipelineLayout();
		void createGraphicsPipeline(VkRenderPass renderPass);
		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects);
		void cleanupGraphicsPipeline();

	private:
		Devices& m_devices;
		VkPipelineLayout m_pipelineLayout;
		std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
	};

} // namespace AE
#pragma once

#include <iostream>
#include <optional>
#include <memory>

#include "../Utils/AREngineDefines.h"

#include "../Devices.h"
#include "../GameObject.h"
#include "../Camera.h"
#include "../FrameInfo.h"
#include "GraphicsPipeline.h"

namespace AE {

	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(Devices& devices) : m_devices{ devices } {}

		// Not copyable or movable
		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem(SimpleRenderSystem&&) = delete;
		SimpleRenderSystem& operator=(SimpleRenderSystem&&) = delete;

		void createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
		//void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
		void createGraphicsPipeline(VkRenderPass renderPass);
		void renderGameObjects(FrameInfo& frameInfo);
		void cleanupGraphicsPipeline();

	private:
		Devices& m_devices;
		VkPipelineLayout m_pipelineLayout;
		std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
	};

} // namespace AE
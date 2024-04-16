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

	class PointLightSystem {
	public:
		PointLightSystem(Devices& devices) : m_devices{ devices } {}

		// Not copyable or movable
		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem& operator=(const PointLightSystem&) = delete;
		PointLightSystem(PointLightSystem&&) = delete;
		PointLightSystem& operator=(PointLightSystem&&) = delete;

		void createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
		void createGraphicsPipeline(VkRenderPass renderPass);
		void update(FrameInfo& frameInfo, GlobalUBO& ubo);
		void render(FrameInfo& frameInfo);
		void cleanupGraphicsPipeline();

	private:
		Devices& m_devices;
		VkPipelineLayout m_pipelineLayout;
		std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
	};

} // namespace AE
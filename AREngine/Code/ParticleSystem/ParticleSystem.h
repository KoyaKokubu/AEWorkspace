#pragma once

#include <iostream>
#include <optional>
#include <memory>

#include "../Utils/AREngineDefines.h"

#include "PointCloud.h"
#include "ComputePipeline.h"
#include "../Devices.h"
#include "../GameObject.h"
#include "../Camera.h"
#include "../FrameInfo.h"
#include "../RenderSystem/GraphicsPipeline.h"

namespace AE {

	class ParticleSystem {
	public:
		ParticleSystem(Devices& devices) : m_devices{ devices } {}

		// Not copyable or movable
		ParticleSystem(const ParticleSystem&) = delete;
		ParticleSystem& operator=(const ParticleSystem&) = delete;
		ParticleSystem(ParticleSystem&&) = delete;
		ParticleSystem& operator=(ParticleSystem&&) = delete;

		void loadPointCloud();
		void createComputePipelineLayout(std::vector<VkDescriptorSetLayout> computeDescriptorSetLayouts);
		void createComputePipeline(VkRenderPass renderPass);
		void createGraphicsPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout);
		void createGraphicsPipeline(VkRenderPass renderPass);
		void dispatch(FrameInfo& frameInfo);
		void renderPointCloud(FrameInfo& frameInfo);
		void cleanupParticleSystem();

		PointCloud& getPointCloud() { return m_pointCloud; }

	private:
		Devices& m_devices;
		VkPipelineLayout m_computePipelineLayout;
		std::unique_ptr<ComputePipeline> m_computePipeline;
		VkPipelineLayout m_graphicsPipelineLayout;
		std::unique_ptr<GraphicsPipeline> m_graphicsPipeline;
		PointCloud m_pointCloud{ m_devices };
	};

} // namespace AE
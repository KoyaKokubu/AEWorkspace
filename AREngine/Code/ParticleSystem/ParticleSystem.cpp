#include <cassert>
#include <array>

#include "../Utils/AREngineIncludes.h"
#include "ParticleSystem.h"

namespace AE {

	void ParticleSystem::loadPointCloud() {
		const float pMean(0.0f);
		const float pDeviation(0.3f);
		m_pointCloud.createVertexBuffers();
		m_pointCloud.createIndexBuffers();
		//m_pointCloud.createParticleModel();
		m_pointCloud.generatePointCloud(pMean, pDeviation);
		m_pointCloud.createSBOObuffers();
	}

	void ParticleSystem::cleanupParticleSystem() {
		m_pointCloud.cleanUpPointCloud();
		vkDestroyPipeline(m_devices.getLogicalDevice(), m_computePipeline->getComputePipeline(), nullptr);
		vkDestroyPipelineLayout(m_devices.getLogicalDevice(), m_computePipelineLayout, nullptr);
		vkDestroyPipeline(m_devices.getLogicalDevice(), m_graphicsPipeline->getGraphicsPipeline(), nullptr);
		vkDestroyPipelineLayout(m_devices.getLogicalDevice(), m_graphicsPipelineLayout, nullptr);
	}

	void ParticleSystem::createComputePipelineLayout(std::vector<VkDescriptorSetLayout> computeDescriptorSetLayouts) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(computeDescriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = computeDescriptorSetLayouts.data();

		if (vkCreatePipelineLayout(m_devices.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_computePipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline layout!");
		}
	}

	void ParticleSystem::createComputePipeline(VkRenderPass renderPass) {
		assert(m_computePipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		m_computePipeline = std::make_unique<ComputePipeline>(m_devices, PARTICLE_COMPUTE_COMPILER_PATH);
		m_computePipeline->createComputePipeline(PARTICLE_COMPUTE_SHADER_PATH, m_computePipelineLayout);
	}

	// "uniform" values in shaders, which are globals similar to dynamic state variables that can be changed at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to pass the 
	// (ex) transformation matrix, texture samplers
	void ParticleSystem::createGraphicsPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout) {
		// index indicates set number
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalDescriptorSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(m_devices.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_graphicsPipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void ParticleSystem::createGraphicsPipeline(VkRenderPass renderPass) {
		assert(m_graphicsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		GraphicsPipeline::defaultPipelineConfig(pipelineConfig);
		//GraphicsPipeline::enableAlphaBlending(pipelineConfig);
		//pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		pipelineConfig.bindingDescriptions = PointCloud::ParticleVertex::getBindingDescriptions();
		pipelineConfig.attributeDescriptions = PointCloud::ParticleVertex::getAttributeDescriptions();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_graphicsPipelineLayout;
		m_graphicsPipeline = std::make_unique<GraphicsPipeline>(m_devices, PARTICLE_GRAPHICS_COMPILER_PATH);
		m_graphicsPipeline->createGraphicsPipeline(PARTICLE_VERT_SHADER_PATH, PARTICLE_FRAG_SHADER_PATH, pipelineConfig);
	}

	void ParticleSystem::dispatch(FrameInfo& frameInfo) {
		m_computePipeline->bind(frameInfo.m_commandBuffer);
		vkCmdBindDescriptorSets(
			frameInfo.m_commandBuffer,
			VK_PIPELINE_BIND_POINT_COMPUTE, 
			m_computePipelineLayout, 
			0, 
			1,
			&frameInfo.m_descriptorSets[0],
			0, 
			nullptr
		);
		
		vkCmdDispatch(frameInfo.m_commandBuffer, POINT_CLOUD_NUM * PARTICLE_NUM / 200, 1, 1);
	}

	void ParticleSystem::renderPointCloud(FrameInfo& frameInfo) {
		m_graphicsPipeline->bind(frameInfo.m_commandBuffer);

		// Bind the descriptor set to the pipeline
		// Since this is called outside the for loop below, 
		// all game objects can refer to the global UBO struct values without rebinding
		vkCmdBindDescriptorSets(
			frameInfo.m_commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_graphicsPipelineLayout,
			0, // first set number. 
			   // If we want to bind a new set and it can be added to the end,
			   // existing sets would not be rebinded by setting the last index here. 
			   // This is why frequently shared sets should occupy the earlier set numbers.
			1, // descriptor set count
			&frameInfo.m_descriptorSets[0],
			0, // can be used for specifying dynamic offsets
			nullptr // can be used for specifying dynamic offsets
		);
		m_pointCloud.bind(frameInfo);
		m_pointCloud.draw(frameInfo.m_commandBuffer);
	}

} // namespace AE
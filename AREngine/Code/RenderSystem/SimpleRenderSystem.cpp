#include <cassert>
#include <array>

#include "../Utils/AREngineIncludes.h"
#include "SimpleRenderSystem.h"

namespace AE {

	// must match the order specified in shaders
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f }; // default: identity matrix
		glm::mat4 normalMatrix{ 1.f };
	};

	void SimpleRenderSystem::cleanupGraphicsPipeline() {
		vkDestroyPipeline(m_devices.getLogicalDevice(), m_graphicsPipeline->getGraphicsPipeline(), nullptr);
		vkDestroyPipelineLayout(m_devices.getLogicalDevice(), m_pipelineLayout, nullptr);
	}

	// "uniform" values in shaders, which are globals similar to dynamic state variables that can be changed at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to pass the 
	// (ex) transformation matrix, texture samplers
	void SimpleRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		// index indicates set number
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalDescriptorSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_devices.getLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void SimpleRenderSystem::createGraphicsPipeline(VkRenderPass renderPass) {
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		GraphicsPipeline::defaultPipelineConfig(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_graphicsPipeline = std::make_unique<GraphicsPipeline>(m_devices, SIMPLE_SHADER_COMPILER_PATH);
		m_graphicsPipeline->createGraphicsPipeline(SIMPLE_VERT_SHADER_PATH, SIMPLE_FRAG_SHADER_PATH, pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
		m_graphicsPipeline->bind(frameInfo.m_commandBuffer);

		// Bind the descriptor set to the pipeline
		// Since this is called outside the for loop below, 
		// all game objects can refer to the global UBO struct values without rebinding
		vkCmdBindDescriptorSets(
			frameInfo.m_commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			0, // first set number. 
			   // If we want to bind a new set and it can be added to the end,
			   // existing sets would not be rebinded by setting the last index here. 
			   // This is why frequently shared sets should occupy the earlier set numbers.
			1, // descriptor set count
			&frameInfo.m_globalDescriptorSet,
			0, // can be used for specifying dynamic offsets
			nullptr // can be used for specifying dynamic offsets
		);

		for (auto& kv : frameInfo.m_gameObjects) {
			GameObject& obj = kv.second;
			if (obj.m_model == nullptr) {
				continue;
			}

			SimplePushConstantData push{};
			push.modelMatrix = obj.m_transformMat.mat4();
			push.normalMatrix = obj.m_transformMat.normalMatrix();

			vkCmdPushConstants(
				frameInfo.m_commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);

			obj.m_model->bind(frameInfo.m_commandBuffer);
			obj.m_model->draw(frameInfo.m_commandBuffer);
		}
	}

} // namespace AE
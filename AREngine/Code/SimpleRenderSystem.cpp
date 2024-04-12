#include <cassert>
#include <map>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "SimpleRenderSystem.h"

namespace AE {

	// must match the order specified in shaders
	struct SimplePushConstantData {
		glm::mat2 transform{ 1.f }; // default: identity matrix
		glm::vec2 offset;
		alignas(16) glm::vec3 color; // GPU memory alignment requirement
	};

	void SimpleRenderSystem::cleanupGraphicsPipeline() {
		vkDestroyPipeline(m_devices.getLogicalDevice(), m_graphicsPipeline->getGraphicsPipeline(), nullptr);
		vkDestroyPipelineLayout(m_devices.getLogicalDevice(), m_pipelineLayout, nullptr);
	}

	// "uniform" values in shaders, which are globals similar to dynamic state variables that can be changed at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to pass the 
	// (ex) transformation matrix, texture samplers
	void SimpleRenderSystem::createPipelineLayout() {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // 
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
		m_graphicsPipeline = std::make_unique<GraphicsPipeline>(m_devices, SYSTEM_FILE_PATH);
		m_graphicsPipeline->createGraphicsPipeline(VERT_SHADER_PATH, FRAG_SHADER_PATH, pipelineConfig);
	}

	void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects) {
		int i = 0;
		for (GameObject& obj : gameObjects) {
			i += 1;
			obj.m_transform2dMat.rotation = glm::mod<float>(obj.m_transform2dMat.rotation + 0.001f * i, 2.f * glm::pi<float>());
		}

		m_graphicsPipeline->bind(commandBuffer);
		for (GameObject& obj : gameObjects) {
			SimplePushConstantData push{};
			push.offset = obj.m_transform2dMat.translation;
			push.color = obj.m_color;
			push.transform = obj.m_transform2dMat.mat2();

			vkCmdPushConstants(
				commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push
			);

			obj.m_model->bind(commandBuffer);
			obj.m_model->draw(commandBuffer);
		}
	}

} // namespace AE
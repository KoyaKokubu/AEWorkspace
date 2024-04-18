#include <cassert>
#include <map>
#include <array>

#include "../Utils/AREngineIncludes.h"
#include "PointLightSystem.h"

namespace AE {

	struct PointLightPushConstants {
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	void PointLightSystem::cleanupGraphicsPipeline() {
		vkDestroyPipeline(m_devices.getLogicalDevice(), m_graphicsPipeline->getGraphicsPipeline(), nullptr);
		vkDestroyPipelineLayout(m_devices.getLogicalDevice(), m_pipelineLayout, nullptr);
	}

	// "uniform" values in shaders, which are globals similar to dynamic state variables that can be changed at drawing time to alter the behavior of your shaders without having to recreate them. They are commonly used to pass the 
	// (ex) transformation matrix, texture samplers
	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalDescriptorSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

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

	void PointLightSystem::createGraphicsPipeline(VkRenderPass renderPass) {
		assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		GraphicsPipeline::defaultPipelineConfig(pipelineConfig);
		GraphicsPipeline::enableAlphaBlending(pipelineConfig);
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_pipelineLayout;
		m_graphicsPipeline = std::make_unique<GraphicsPipeline>(m_devices, POINT_SHADER_COMPILER_PATH);
		m_graphicsPipeline->createGraphicsPipeline(POINT_LIGHT_VERT_SHADER_PATH, POINT_LIGHT_FRAG_SHADER_PATH, pipelineConfig);
	}

	void PointLightSystem::update(FrameInfo& frameInfo, GlobalUBO& ubo) {
		glm::highp_mat4 rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.m_frameTime, { 0.f, -1.f, 0.f });
		int lightIndex = 0;
		for (auto& kv : frameInfo.m_gameObjects) {
			GameObject& obj = kv.second;
			if (obj.m_pointLight == nullptr) {
				continue;
			}

			assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

			// update light position
			obj.m_transformMat.m_translation = glm::vec3(rotateLight * glm::vec4(obj.m_transformMat.m_translation, 1.f));

			// copy light to ubo
			ubo.pointLights[lightIndex].position = glm::vec4(obj.m_transformMat.m_translation, 1.f);
			ubo.pointLights[lightIndex].color = glm::vec4(obj.m_color, obj.m_pointLight->m_lightIntensity);

			lightIndex += 1;
		}
		ubo.numLights = lightIndex;
	}

	void PointLightSystem::render(FrameInfo& frameInfo) {
		// sort lights
		std::map<float, GameObject::u_id> sorted;
		for (auto& kv : frameInfo.m_gameObjects) {
			auto& obj = kv.second;
			if (obj.m_pointLight == nullptr) {
				continue;
			}

			// calculate distance from light to camera
			auto offset = frameInfo.m_camera.getPosition() - obj.m_transformMat.m_translation;
			float disSquared = glm::dot(offset, offset);
			sorted[disSquared] = obj.getId();
		}

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
			&frameInfo.m_descriptorSets[0],
			0, // can be used for specifying dynamic offsets
			nullptr // can be used for specifying dynamic offsets
		);

		// iterate through sorted lights in reverse order
		for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
			auto& obj = frameInfo.m_gameObjects.at(it->second);

			PointLightPushConstants push{};
			push.position = glm::vec4(obj.m_transformMat.m_translation, 1.f);
			push.color = glm::vec4(obj.m_color, obj.m_pointLight->m_lightIntensity);
			push.radius = obj.m_transformMat.m_scale.x;

			vkCmdPushConstants(
				frameInfo.m_commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push
			);
			vkCmdDraw(frameInfo.m_commandBuffer, 6, 1, 0, 0);
		}
	}

} // namespace AE
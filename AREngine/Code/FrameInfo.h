#pragma once

#include "Utils/AREngineIncludes.h"
#include "Utils/AREngineDefines.h"
#include "Camera.h"
#include "GameObject.h"

namespace AE {

	struct PointLightGPU {
		glm::vec4 position{};  // ignore w
		glm::vec4 color{};     // w is intensity
	};

	// Global Uniform Buffer Object
	struct GlobalUBO {
		glm::mat4 projection{ 1.f };
		glm::mat4 view{ 1.f };
		glm::mat4 inverseView{ 1.f };
		glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f }; // w is instensity
		PointLightGPU pointLights[MAX_LIGHTS];
		int numLights;
	};

	struct FrameInfo {
		int m_frameIndex;
		float m_frameTime;
		VkCommandBuffer m_commandBuffer;
		Camera& m_camera;
		//VkDescriptorSet m_globalDescriptorSet;
		std::vector<VkDescriptorSet> m_descriptorSets;
		GameObject::Map& m_gameObjects;
	};

} // namespace AE
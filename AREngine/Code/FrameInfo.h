#pragma once

#include "Utils/AREngineIncludes.h"
#include "Camera.h"
#include "GameObject.h"

namespace AE {
	struct FrameInfo {
		int m_frameIndex;
		float m_frameTime;
		VkCommandBuffer m_commandBuffer;
		Camera& m_camera;
		VkDescriptorSet m_globalDescriptorSet;
		GameObject::Map& m_gameObjects;
	};
} // namespace AE
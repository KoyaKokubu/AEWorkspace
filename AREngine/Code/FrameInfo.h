#pragma once

#include "Utils/AREngineIncludes.h"
#include "Camera.h"

namespace AE {
	struct FrameInfo {
		int m_frameIndex;
		float m_frameTime;
		VkCommandBuffer m_commandBuffer;
		Camera& m_camera;
		VkDescriptorSet globalDescriptorSet;
	};
} // namespace AE
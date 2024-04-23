#pragma once

#include "../../Utils/AREngineIncludes.h"
#include "../../Camera.h"
#include "../../ParticleSystem/ParticleSystem.h"

namespace AE {

	class RGBDvision {
	public:
		RGBDvision(ParticleSystem& particleSystem)
			: m_particleSystem{ particleSystem }
		{}

		void setCameraExternalParameters();
		void generatePointCloud();

		void setViewPose(glm::vec3 position, glm::mat4 rotationMat);

	private:
		ParticleSystem& m_particleSystem;
		std::vector<cv::Mat> m_colorImgs, m_depthImgs;
		std::vector<glm::vec3> m_cameraPos;
		std::vector<glm::mat4> m_cameraRotation;
		glm::mat4 m_inverseViewMatrix{ 1.f };
	};

} // namespace AE
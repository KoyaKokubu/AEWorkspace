#include <cassert>
#include <limits>

#include "Camera.h"

namespace AE {

	void Camera::setOrthographicProjection(
		float left, float right, float top, float bottom, float near, float far) {
		m_projectionMatrix = glm::mat4{ 1.0f };
		m_projectionMatrix[0][0] = 2.f / (right - left);
		m_projectionMatrix[1][1] = 2.f / (bottom - top);
		m_projectionMatrix[2][2] = 1.f / (far - near);
		m_projectionMatrix[3][0] = -(right + left) / (right - left);
		m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		m_projectionMatrix[3][2] = -near / (far - near);
	}

	void Camera::setPerspectiveProjection(float vertical_fov, float aspect, float near, float far) {
		assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
		const float tanHalfFov = tan(vertical_fov / 2.f);
		m_projectionMatrix = glm::mat4{ 0.0f };
		m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFov);
		m_projectionMatrix[1][1] = 1.f / (tanHalfFov);
		m_projectionMatrix[2][2] = far / (far - near);
		m_projectionMatrix[2][3] = 1.f;
		m_projectionMatrix[3][2] = -(far * near) / (far - near);
	}

	void Camera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
		const glm::vec3 w{ glm::normalize(direction) };
		const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
		const glm::vec3 v{ glm::cross(w, u) };

		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);
	}

	void Camera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
		setViewDirection(position, target - position, up);
	}

	void Camera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
		const float cos3 = glm::cos(rotation.z);
		const float sin3 = glm::sin(rotation.z);
		const float cos2 = glm::cos(rotation.x);
		const float sin2 = glm::sin(rotation.x);
		const float cos1 = glm::cos(rotation.y);
		const float sin1 = glm::sin(rotation.y);
		const glm::vec3 u{ (cos1 * cos3 + sin1 * sin2 * sin3), (cos2 * sin3), (cos1 * sin2 * sin3 - cos3 * sin1) };
		const glm::vec3 v{ (cos3 * sin1 * sin2 - cos1 * sin3), (cos2 * cos3), (cos1 * cos3 * sin2 + sin1 * sin3) };
		const glm::vec3 w{ (cos2 * sin1), (-sin2), (cos1 * cos2) };
		m_viewMatrix = glm::mat4{ 1.f };
		m_viewMatrix[0][0] = u.x;
		m_viewMatrix[1][0] = u.y;
		m_viewMatrix[2][0] = u.z;
		m_viewMatrix[0][1] = v.x;
		m_viewMatrix[1][1] = v.y;
		m_viewMatrix[2][1] = v.z;
		m_viewMatrix[0][2] = w.x;
		m_viewMatrix[1][2] = w.y;
		m_viewMatrix[2][2] = w.z;
		m_viewMatrix[3][0] = -glm::dot(u, position);
		m_viewMatrix[3][1] = -glm::dot(v, position);
		m_viewMatrix[3][2] = -glm::dot(w, position);
	}

} //namespace AE
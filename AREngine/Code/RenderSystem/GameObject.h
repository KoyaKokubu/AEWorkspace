#pragma once

#include <memory>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"

namespace AE {

    struct TransformMat {
        glm::vec3 m_translation{};  // (position offset)
        glm::vec3 m_scale{ 1.f, 1.f, 1.f };
        glm::vec3 m_rotation{};

        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4() {
            const float cos3 = glm::cos(m_rotation.z);
            const float sin3 = glm::sin(m_rotation.z);
            const float cos2 = glm::cos(m_rotation.x);
            const float sin2 = glm::sin(m_rotation.x);
            const float cos1 = glm::cos(m_rotation.y);
            const float sin1 = glm::sin(m_rotation.y);
            return glm::mat4{
                {
                    m_scale.x * (cos1 * cos3 + sin1 * sin2 * sin3),
                    m_scale.x * (cos2 * sin3),
                    m_scale.x * (cos1 * sin2 * sin3 - cos3 * sin1),
                    0.0f,
                },
                {
                    m_scale.y * (cos3 * sin1 * sin2 - cos1 * sin3),
                    m_scale.y * (cos2 * cos3),
                    m_scale.y * (cos1 * cos3 * sin2 + sin1 * sin3),
                    0.0f,
                },
                {
                    m_scale.z * (cos2 * sin1),
                    m_scale.z * (-sin2),
                    m_scale.z * (cos1 * cos2),
                    0.0f,
                },
                {m_translation.x, m_translation.y, m_translation.z, 1.0f} };
        }
    };
	
	class GameObject {
    public:
        using u_id = unsigned int;

        static GameObject createGameObject() {
            static u_id currentId = 0;
            return GameObject{ currentId++ };
        }

        GameObject(const GameObject&) = delete;
        GameObject& operator=(const GameObject&) = delete;
        GameObject(GameObject&&) = default;
        GameObject& operator=(GameObject&&) = default;

        const u_id getId() const { return m_id; }

        std::shared_ptr<Model> m_model{};
        glm::vec3 m_color{};
        TransformMat m_transformMat{};

    private:
        GameObject(u_id objId) : m_id{ objId } {}

        u_id m_id;
	};

} // namespace AE
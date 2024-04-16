#pragma once

#include <memory>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>

#include "Model.h"

namespace AE {

    struct TransformMat {
        glm::vec3 m_translation{};  // (position offset)
        glm::vec3 m_scale{ 1.f, 1.f, 1.f };
        glm::vec3 m_rotation{};

        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();
        glm::mat4 normalMatrix();
    };
	
	class GameObject {
    public:
        using u_id = unsigned int;
        using Map = std::unordered_map<u_id, GameObject>;

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
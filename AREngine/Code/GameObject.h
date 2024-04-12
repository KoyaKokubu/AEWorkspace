#pragma once

#include <memory>

#include "Model.h"

namespace AE {

    struct Transform2dMat {
        glm::vec2 translation{};  // (position offset)
        glm::vec2 scale{ 1.f, 1.f };
        float rotation;

        glm::mat2 mat2() {
            const float sin = glm::sin(rotation);
            const float cos = glm::cos(rotation);
            glm::mat2 rotationMat{ 
                {cos, sin}, 
                {-sin, cos} 
            };
            glm::mat2 scaleMat{ 
                {scale.x, .0f}, 
                {.0f, scale.y} 
            };
            return rotationMat * scaleMat;
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

        u_id getId() { return m_id; }

        std::shared_ptr<Model> m_model{};
        glm::vec3 m_color{};
        Transform2dMat m_transform2dMat{};

    private:
        GameObject(u_id objId) : m_id{ objId } {}

        u_id m_id;
	};

} // namespace AE
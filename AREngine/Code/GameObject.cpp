#include "GameObject.h"

namespace AE {

    glm::mat4 TransformMat::mat4() {
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
            {m_translation.x, m_translation.y, m_translation.z, 1.0f}
        };
    }

    glm::mat4 TransformMat::normalMatrix() {
        const float cos3 = glm::cos(m_rotation.z);
        const float sin3 = glm::sin(m_rotation.z);
        const float cos2 = glm::cos(m_rotation.x);
        const float sin2 = glm::sin(m_rotation.x);
        const float cos1 = glm::cos(m_rotation.y);
        const float sin1 = glm::sin(m_rotation.y);
        const glm::vec3 inverseScale = 1.0f / m_scale;

        return glm::mat3{
            {
                inverseScale.x * (cos1 * cos3 + sin1 * sin2 * sin3),
                inverseScale.x * (cos2 * sin3),
                inverseScale.x * (cos1 * sin2 * sin3 - cos3 * sin1),
            },
            {
                inverseScale.y * (cos3 * sin1 * sin2 - cos1 * sin3),
                inverseScale.y * (cos2 * cos3),
                inverseScale.y * (cos1 * cos3 * sin2 + sin1 * sin3),
            },
            {
                inverseScale.z * (cos2 * sin1),
                inverseScale.z * (-sin2),
                inverseScale.z * (cos1 * cos2),
            }
        };
    }

} // namespace AE
#pragma once

#include "../Utils/AREngineIncludes.h"
#include "../Devices.h"
#include "../Buffer.h"
#include "../Model.h"
#include "../FrameInfo.h"

#define INSTANCING_INDIRECT_DRAW

namespace AE {

    class PointCloud {
    public:
        struct ParticleVertex {
            ParticleVertex() {};
            ParticleVertex(float px, float py, float pz) : position{ px, py, pz } {}

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            glm::vec3 position;
        };

        struct ParticleInstance {
            ParticleInstance() {};
            ParticleInstance(
                float px, float py, float pz,
                float cx = 1.f, float cy = 1.f, float cz = 1.f, float ca = 1.0f,
                float vx = 0.f, float vy = 0.f, float vz = 1.f
            )
                : position{ px, py, pz, 1.0f }
                , color{ cx, cy, cz, ca }
                , velocity{ vx, vy, vz, 0.0f }
            {}

            ParticleInstance(
                glm::vec4 pos, 
                glm::vec4 color,
                float vx = 0.f, float vy = 0.f, float vz = 1.f
            )
                : position{ pos }
                , color{ color }
                , velocity{ vx, vy, vz, 0.0f }
            {}

            // static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            // static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            glm::vec4 position;
            glm::vec4 color;
            glm::vec4 velocity;
        };

        PointCloud(Devices& devices) : m_devices{ devices } {};
        void cleanUpPointCloud() {
            m_indirectCommands.clear();
            m_sbooBuffer.clear();
            m_particles.clear();
            m_particleModel = nullptr;
            m_vertexBuffer = nullptr;
            m_indexBuffer = nullptr;
            m_indirectCommandsBuffer.clear();
        };

        PointCloud(const PointCloud& pointcCloud) = delete;
        PointCloud& operator=(const PointCloud& pointCloud) = delete;

        void generatePointCloud(int pointCloudNum, int particleNum, float mean, float deviation);
        void setPointCloud(
            std::vector<std::vector<glm::vec4>> pointCloud_position,
            std::vector<std::vector<glm::vec4>> pointCloud_color
        );
        void createVertexBuffers();
        void createIndexBuffers();
        void createParticleModel();
        void createSBOObuffers();
        void createIndirectBuffers(int pointCloudNum, std::vector<int> particleNum);
        void bind(FrameInfo& frameInfo);
        void draw(FrameInfo& frameInfo);

        std::vector<std::unique_ptr<Buffer>>& getSBOObuffers() { return m_sbooBuffer; };
        std::vector<std::unique_ptr<Buffer>>& getIndirectCommandsBuffers() { return m_indirectCommandsBuffer; };

    private:
        Devices& m_devices;
        std::vector<std::unique_ptr<Buffer>> m_sbooBuffer;
        std::unique_ptr<Buffer> m_vertexBuffer;
        std::unique_ptr<Buffer> m_indexBuffer;
        uint32_t m_indexCount;
        std::vector<std::unique_ptr<Buffer>> m_indirectCommandsBuffer;
        std::vector<VkDrawIndexedIndirectCommand> m_indirectCommands;
        uint32_t m_indirectDrawCount;

        std::vector<ParticleVertex> m_vertices;
        uint32_t m_vertexCount;
        std::vector<ParticleInstance> m_particles;
        uint32_t m_particleCount;
        std::unique_ptr<Model> m_particleModel;
    };

} // namespace AE
#include <random>

#include "../Utils/AREngineDefines.h"
#include "PointCloud.h"

namespace AE {

    void PointCloud::generatePointCloud(float mean, float deviation) {
        const float range(1.5f);
        std::mt19937 rn(54321);
        std::uniform_real_distribution center(-range, range);
        std::uniform_real_distribution uniform(0.0f, 2.0f);
        std::normal_distribution normal(mean, deviation);
        m_particles.reserve(POINT_CLOUD_NUM * PARTICLE_NUM);
        for (int i = 0; i < POINT_CLOUD_NUM; ++i) {
            const float cx(center(rn)), cy(center(rn)), cz(center(rn));
            for (int j = 0; j < PARTICLE_NUM; ++j) {
                const float cp(uniform(rn) - 1.0f);
                const float sp(sqrt(1.0f - cp * cp));
                const float t(3.1415927f * uniform(rn));
                const float ct(cos(t)), st(sin(t));
                const float r(normal(rn));
                m_particles.emplace_back(r * sp * ct + cx, r * sp * st + cy, r * cp + cz);
            }
        }
    }

    void PointCloud::createParticleModel() {
        m_particleModel = Model::createModelFromFile(m_devices, "Models/particle_quad.obj");
    }

    void PointCloud::createVertexBuffers() {
        std::vector<glm::vec3> vertices{
            {-1.f, -1.f, 0.f},
            {-1.f, 1.f, 0.f},
            {1.f, 1.f, 0.f},
            {1.f, -1.f, 0.f}
        };

        for (int i = 0; i < vertices.size(); i++) {
            m_vertices.emplace_back(vertices[i].x, vertices[i].y, vertices[i].z);
        }

        m_vertexCount = static_cast<uint32_t>(m_vertices.size());
        assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertexCount;
        uint32_t vertexSize = sizeof(m_vertices[0]);

        Buffer stagingBuffer{
            m_devices,
            vertexSize,
            m_vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        // create a region of host(cpu) memory mapped to device memory and set data to point to the beginning of the mapped memory range
        stagingBuffer.map();
        // As we specified VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data will automatically be flushed to update device memory after memcpy
        stagingBuffer.writeToBuffer((void*)m_vertices.data());

        m_vertexBuffer = std::make_unique<Buffer>(
            m_devices,
            vertexSize,
            m_vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_devices.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
    }

    void PointCloud::createIndexBuffers() {
        std::vector<uint32_t> indices{ 0, 2, 1, 2, 0, 3};

        m_indexCount = static_cast<uint32_t>(indices.size());

        VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        Buffer stagingBuffer{
            m_devices,
            indexSize,
            m_indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());

        m_indexBuffer = std::make_unique<Buffer>(
            m_devices,
            indexSize,
            m_indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_devices.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
    }

    void PointCloud::createSBOObuffers() {
        m_particleCount = static_cast<uint32_t>(m_particles.size());

        VkDeviceSize bufferSize = sizeof(m_particles[0]) * m_particleCount;
        uint32_t vertexSize = sizeof(m_particles[0]);
        Buffer stagingBuffer{
            m_devices,
            vertexSize,
            m_particleCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)m_particles.data());

        m_sbooBuffer.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_sbooBuffer[i] = std::make_unique<Buffer>(
                m_devices,
                vertexSize,
                m_particleCount,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, // | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT 
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            m_devices.copyBuffer(stagingBuffer.getBuffer(), m_sbooBuffer[i]->getBuffer(), bufferSize);
        }
    }

    void PointCloud::createIndirectBuffers() {
        m_indirectCommands.clear();

        //int instanceNum = POINT_CLOUD_NUM * PARTICLE_NUM;
        for (int i=0; i < POINT_CLOUD_NUM; i++) {
            VkDrawIndexedIndirectCommand indirectCmd{};
            indirectCmd.instanceCount = PARTICLE_NUM;
            indirectCmd.firstInstance = i * PARTICLE_NUM;
            indirectCmd.firstIndex = 0;
            indirectCmd.indexCount = m_indexCount;

            m_indirectCommands.push_back(indirectCmd);
        }

        m_indirectDrawCount = static_cast<uint32_t>(m_indirectCommands.size());
        VkDeviceSize bufferSize = sizeof(m_indirectCommands[0]) * m_indirectDrawCount;
        uint32_t elementSize = sizeof(m_indirectCommands[0]);

        Buffer stagingBuffer{
            m_devices,
            elementSize,
            m_indirectDrawCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)m_indirectCommands.data());

        m_indirectCommandsBuffer.resize(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_indirectCommandsBuffer[i] = std::make_unique<Buffer>(
                m_devices,
                elementSize,
                m_indirectDrawCount,
                VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );
            m_devices.copyBuffer(stagingBuffer.getBuffer(), m_indirectCommandsBuffer[i]->getBuffer(), bufferSize);
        }
    }

    // Record to command buffer to bind one vertex buffer starting at binding zero
    void PointCloud::bind(FrameInfo& frameInfo) {
        // We can add multiple bindings by additional elements to the arrays below.
        VkBuffer buffers[] = { m_vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 }; // offset in bindings
        vkCmdBindVertexBuffers(frameInfo.m_commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindIndexBuffer(frameInfo.m_commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void PointCloud::draw(FrameInfo& frameInfo) {
#ifdef INSTANCING_INDIRECT_DRAW
        // Indirect Draw
        // If the multi draw feature is supported:
        // One draw call for an arbitrary number of objects
        // Index offsets and instance count are taken from the indirect buffer
        if (m_devices.getDeviceFeatures().multiDrawIndirect) {
            vkCmdDrawIndexedIndirect(
                frameInfo.m_commandBuffer,
                m_indirectCommandsBuffer[frameInfo.m_frameIndex]->getBuffer(),
                0,
                m_indirectDrawCount,
                sizeof(VkDrawIndexedIndirectCommand)
            );
        }
        else {
            // If multi draw is not available, we must issue separate draw commands
            for (auto j = 0; j < m_indirectCommands.size(); j++)
            {
                vkCmdDrawIndexedIndirect(
                    frameInfo.m_commandBuffer,
                    m_indirectCommandsBuffer[frameInfo.m_frameIndex]->getBuffer(),
                    j * sizeof(VkDrawIndexedIndirectCommand),
                    1,
                    sizeof(VkDrawIndexedIndirectCommand)
                );
            }
        }
#else
        // Instancing Draw
        //m_particleModel->instancingDraw(commandBuffer);
        vkCmdDrawIndexed(frameInfo.m_commandBuffer, m_indexCount, m_particleCount, 0, 0, 0);
#endif
    }

    // binding(s) corresponded to a single vertex buffer
    std::vector<VkVertexInputBindingDescription> PointCloud::ParticleVertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(ParticleVertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> PointCloud::ParticleVertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(ParticleVertex, position) });
        return attributeDescriptions;
    }

} // namespace AE
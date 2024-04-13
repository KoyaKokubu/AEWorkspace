#pragma once

#include "../Utils/AREngineIncludes.h"
#include "../Devices.h"

namespace AE {

    // Take vertex data created by or write in a file on the cpu
    // Then, allocate the memory and copy the data over a GPU to render efficiently
    class Model {
    public:
        struct Vertex {
            glm::vec3 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        Model(Devices& devices) : m_devices{ devices } {};

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        const VkBuffer& getVertexBuffer() const { return m_vertexBuffer;  }
        const VkDeviceMemory& getVertexBufferMemory() const { return m_vertexBufferMemory; }

    private:
        Devices& m_devices;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        uint32_t m_vertexCount;
    };

}  // namespace AE
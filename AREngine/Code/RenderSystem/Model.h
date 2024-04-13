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

        struct Builder {
            std::vector<Vertex> m_vertices{};
            std::vector<uint32_t> m_indices{};
        };

        Model(Devices& devices) : m_devices{ devices } {};

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        const VkBuffer& getVertexBuffer() const { return m_vertexBuffer;  }
        const VkDeviceMemory& getVertexBufferMemory() const { return m_vertexBufferMemory; }
        const VkBuffer& getIndexBuffer() const { return m_indexBuffer; }
        const VkDeviceMemory& getIndexBufferMemory() const { return m_indexBufferMemory; }

        bool m_hasIndexBuffer = false;

    private:
        Devices& m_devices;

        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        uint32_t m_vertexCount;

        VkBuffer m_indexBuffer;
        VkDeviceMemory m_indexBufferMemory;
        uint32_t m_indexCount;
    };

}  // namespace AE
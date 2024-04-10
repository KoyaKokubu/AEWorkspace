#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // expect depth buffer values to range from 0 to 1
#include <glm/glm.hpp>
#include <vector>

#include "Devices.h"

namespace AE {

    // Take vertex data created by or write in a file on the cpu
    // Then, allocate the memory and copy the data over a GPU to render efficiently
    class Model {
    public:
        struct Vertex {
            glm::vec2 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        Model(Devices& devices);

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        VkBuffer& getVertexBuffer() { return m_vertexBuffer;  }
        VkDeviceMemory& getVertexBufferMemory() { return m_vertexBufferMemory; }

    private:
        Devices& m_devices;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBufferMemory;
        uint32_t m_vertexCount;
    };

}  // namespace AE
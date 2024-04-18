#pragma once

#include <memory>

#include "Utils/AREngineIncludes.h"
#include "Devices.h"
#include "Buffer.h"
#include "Texture.h"

namespace AE {

    // Take vertex data created by or write in a file on the cpu
    // Then, allocate the memory and copy the data over a GPU to render efficiently
    class Model {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex& other) const {
                return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
            }
        };

        struct Builder {
            std::vector<Vertex> m_vertices{};
            std::vector<uint32_t> m_indices{};

            void loadModel(const char* filePath);
        };

        Model(Devices& devices) : m_devices{ devices } {};

        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        static std::unique_ptr<Model> createModelFromFile(Devices& devices, const char* filePath);

        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);
        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        void createTexture(const char* filePath);

        bool m_hasIndexBuffer = false;
        bool m_hasTexture = false;
        std::unique_ptr<Texture> m_texture;

    private:
        Devices& m_devices;

        std::unique_ptr<Buffer> m_vertexBuffer;
        uint32_t m_vertexCount;

        std::unique_ptr<Buffer> m_indexBuffer;
        uint32_t m_indexCount;
    };

}  // namespace AE
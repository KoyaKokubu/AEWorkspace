#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Model.h"
#include "Utils/AREngineDefines.h"
#include "Utils/utils.h"
#include "Input/tiny_obj_loader.h"

namespace std {
    template <>
    struct hash<AE::Model::Vertex> {
        size_t operator()(AE::Model::Vertex const& vertex) const {
            size_t seed = 0;
            AE::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}  // namespace std

namespace AE {

    std::unique_ptr<Model> Model::createModelFromFile(Devices& devices, const char* filePath) {
        Builder builder{};
        builder.loadModel(filePath);
        //printf("Vertex count: %d\n", builder.m_vertices.size());
        std::unique_ptr<Model> new_model = std::make_unique<Model>(devices);
        new_model->createVertexBuffers(builder.m_vertices);
        new_model->createIndexBuffers(builder.m_indices);
        return new_model;
    }

    void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

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
        stagingBuffer.writeToBuffer((void*)vertices.data());

        m_vertexBuffer = std::make_unique<Buffer>(
            m_devices,
            vertexSize,
            m_vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_devices.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
    }

    void Model::createIndexBuffers(const std::vector<uint32_t>& indices) {
        m_indexCount = static_cast<uint32_t>(indices.size());
        m_hasIndexBuffer = m_indexCount > 0;
        if (!m_hasIndexBuffer) {
            return; 
        }

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

    void Model::createTexture(const char* filePath) {
        m_texture = std::make_unique<Texture>(m_devices);
        m_texture->createTextureImage(filePath);
        //m_texture->generateMipmaps();
        m_texture->createTextureImageView();
        m_texture->createTextureSampler();
        m_hasTexture = true;
    }

    void Model::draw(VkCommandBuffer commandBuffer) {
        if (m_hasIndexBuffer) {
            // Parameters: (commandbuffer, indexCount, instanceCount, firstVertex, vertex offset, firstInstance)
            vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        }
        else {
            // vkCmdDraw(m_commandBuffers[i], vertexCount, instanceCount, firstVertex, firstInstance);
            // 2. vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
            // 3. instanceCount: Used for instanced rendering, use 1 if you're not doing that.
            // 4. firstVertex : Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
            // 5. firstInstance : Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
            vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
        }
    }

    // Record to command buffer to bind one vertex buffer starting at binding zero
    void Model::bind(VkCommandBuffer commandBuffer) {
        // We can add multiple bindings by additional elements to the arrays below.
        VkBuffer buffers[] = { m_vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 }; // offset in bindings
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (m_hasIndexBuffer) {
            // Third parameter: initial offset
            vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    // binding(s) corresponded to a single vertex buffer
    std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
        attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
        attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
        attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });
        return attributeDescriptions;

        //attributeDescriptions[0].binding = 0;
        //attributeDescriptions[0].location = 0; // corresponded to the location specified in the vertex shader
        //attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        //attributeDescriptions[0].offset = offsetof(Vertex, position); // offset from the beginning of struct Vertex -> 0 byte
        //attributeDescriptions[1].binding = 0;
        //attributeDescriptions[1].location = 1;
        //attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        //attributeDescriptions[1].offset = offsetof(Vertex, color); // offset from the beginning of struct Vertex -> 8 bytes
        //return attributeDescriptions;
    }

    void Model::Builder::loadModel(const char* filePath) {
        tinyobj::attrib_t attrib; // postition, color, normal, texture coordinate
        std::vector<tinyobj::shape_t> shapes; // index values for each element
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filePath)) {
            throw std::runtime_error(warn + err);
        }

        m_vertices.clear();
        m_indices.clear();

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const tinyobj::shape_t& shape : shapes) {
            for (const tinyobj::index_t& index : shape.mesh.indices) {
                Vertex vertex{};

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2],
                    };
                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2],
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2],
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1],
                    };
                }

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                    m_vertices.emplace_back(vertex);
                }
                m_indices.emplace_back(uniqueVertices[vertex]);
            }
        }
    }

}  // namespace AE
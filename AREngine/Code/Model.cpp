#include <cassert>
#include <cstring>

#include "Model.h"

namespace AE {

    Model::Model(Devices& devices) : m_devices{ devices } {
    }

    void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
        m_vertexCount = static_cast<uint32_t>(vertices.size());
        assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
        m_devices.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_vertexBuffer,
            m_vertexBufferMemory
        );

        void* data;
        // create a region of host(cpu) memory mapped to device memory and set data to point to the beginning of the mapped memory range
        // m_vertexBufferMemory: device memory
        // data: host memory
        vkMapMemory(m_devices.getLogicalDevice(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
        // As we specified VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, data will automatically be flushed to update device memory after memcpy
        // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT uses memory heap.
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(m_devices.getLogicalDevice(), m_vertexBufferMemory);
    }

    void Model::draw(VkCommandBuffer commandBuffer) {
        // vkCmdDraw(m_commandBuffers[i], vertexCount, instanceCount, firstVertex, firstInstance);
        // 2. vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
        // 3. instanceCount: Used for instanced rendering, use 1 if you're not doing that.
        // 4. firstVertex : Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
        // 5. firstInstance : Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
        vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
    }

    // Record to command buffer to bind one vertex buffer starting at binding zero
    void Model::bind(VkCommandBuffer commandBuffer) {
        // We can add multiple bindings by additional elements to the arrays below.
        VkBuffer buffers[] = { m_vertexBuffer };
        VkDeviceSize offsets[] = { 0 }; // offset in bindings
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
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
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0; // corresponded to the location specified in the vertex shader
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position); // offset from the beginning of struct Vertex -> 0 byte
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color); // offset from the beginning of struct Vertex -> 8 bytes
        return attributeDescriptions;
    }

}  // namespace AE
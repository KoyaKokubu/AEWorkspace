#include <cassert>
#include <stdexcept>

#include "Descriptors.h"

namespace AE {

    // *************** Descriptor Set Layout Builder *********************

    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count) 
    {
        assert(m_bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        m_bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::build() const {
        return std::make_unique<DescriptorSetLayout>(m_devices, m_bindings);
    }

    // *************** Descriptor Set Layout *********************

    DescriptorSetLayout::DescriptorSetLayout(Devices& devices, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : m_devices{ devices }
        , m_bindings{ bindings } 
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(m_devices.getLogicalDevice(), &descriptorSetLayoutInfo, nullptr, &m_descriptorSetLayout) 
            != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        vkDestroyDescriptorSetLayout(m_devices.getLogicalDevice(), m_descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
        m_poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags) {
        m_poolFlags = flags;
        return *this;
    }
    DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count) {
        m_maxSets = count;
        return *this;
    }

    std::unique_ptr<DescriptorPool> DescriptorPool::Builder::build() const {
        return std::make_unique<DescriptorPool>(m_devices, m_maxSets, m_poolFlags, m_poolSizes);
    }

    // *************** Descriptor Pool *********************

    DescriptorPool::DescriptorPool(
        Devices& devices,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes
    )
        : m_devices{ devices } 
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(m_devices.getLogicalDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    DescriptorPool::~DescriptorPool() {
        vkDestroyDescriptorPool(m_devices.getLogicalDevice(), m_descriptorPool, nullptr);
    }

    bool DescriptorPool::allocateDescriptorSets(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds a new pool whenever an old pool fills up.
        if (vkAllocateDescriptorSets(m_devices.getLogicalDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void DescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(
            m_devices.getLogicalDevice(),
            m_descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void DescriptorPool::resetPool() {
        vkResetDescriptorPool(m_devices.getLogicalDevice(), m_descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
        : m_setLayout{ setLayout }
        , m_pool{ pool } 
    {}

    DescriptorWriter& DescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(m_setLayout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");

        VkDescriptorSetLayoutBinding& bindingDescription = m_setLayout.m_bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple"
        );

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        m_writes.push_back(write);
        return *this;
    }

    DescriptorWriter& DescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(m_setLayout.m_bindings.count(binding) == 1 && "Layout does not contain specified binding");

        VkDescriptorSetLayoutBinding& bindingDescription = m_setLayout.m_bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple"
        );

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        m_writes.push_back(write);
        return *this;
    }

    bool DescriptorWriter::build(VkDescriptorSet& set) {
        bool success = m_pool.allocateDescriptorSets(m_setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    // write per set. so, it's okay to use the same set for all in m_writes.
    // If a descriptor set has been already created and you just want to update its descriptors, then just call this function.
    void DescriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : m_writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(m_pool.m_devices.getLogicalDevice(), m_writes.size(), m_writes.data(), 0, nullptr);
    }

} // namespace AE
#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Devices.h"

namespace AE {

    class DescriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(Devices& devices) : m_devices{ devices } {}

            Builder& addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType, // can be uniform buffer, image sampler and so on
                VkShaderStageFlags stageFlags,
                uint32_t count = 1
            );
            std::unique_ptr<DescriptorSetLayout> build() const;

        private:
            Devices& m_devices;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings{};
        };

        DescriptorSetLayout(Devices& devices, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~DescriptorSetLayout();

        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return m_descriptorSetLayout; }

    private:
        Devices& m_devices;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_bindings;

        friend class DescriptorWriter;
    };

    class DescriptorPool {
    public:
        class Builder {
        public:
            Builder(Devices& devices) : m_devices{ devices } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count); // tell the size needed ahead of time
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags); // configure the behavior of pool object
            Builder& setMaxSets(uint32_t count); // total number of descriptor sets that can be allocated from this object
            std::unique_ptr<DescriptorPool> build() const;

        private:
            Devices& m_devices;
            std::vector<VkDescriptorPoolSize> m_poolSizes{};
            uint32_t m_maxSets = 1000;
            VkDescriptorPoolCreateFlags m_poolFlags = 0;
        };

        DescriptorPool(
            Devices& devices,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize>& poolSizes
        );
        ~DescriptorPool();

        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        bool allocateDescriptorSets(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;
        void resetPool();

    private:
        Devices& m_devices;
        VkDescriptorPool m_descriptorPool;

        friend class DescriptorWriter;
    };

    // allocate VkDescriptor from the pool and write the necessary informamtion for each descriptor that the set contains.
    class DescriptorWriter {
    public:
        DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);

        DescriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        DescriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        DescriptorSetLayout& m_setLayout;
        DescriptorPool& m_pool;
        std::vector<VkWriteDescriptorSet> m_writes;
    };

} // namespace AE
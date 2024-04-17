#pragma once

#include "../Utils/AREngineIncludes.h"

namespace AE {

	class Devices;

	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class GraphicsPipeline {
	public:
		GraphicsPipeline(Devices& devices, const char* systemFilePath);
		~GraphicsPipeline() {}
		
		GraphicsPipeline(const GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

		void createGraphicsPipeline(const char* vertFilePath, const char* fragFilePath, const PipelineConfigInfo& configInfo);
		static void defaultPipelineConfig(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);
		void bind(VkCommandBuffer commandBuffer);

		const VkPipeline& getGraphicsPipeline() const { return m_graphicsPipeline; }

	private:
		static void compileShaderFiles(const char* systemFilePath);
		static std::vector<char> readFile(const char* filePath);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		Devices& m_devices;
		VkPipeline m_graphicsPipeline;
		VkShaderModule m_vertShaderModule;
		VkShaderModule m_fragShaderModule;
	};

} // namespace AE
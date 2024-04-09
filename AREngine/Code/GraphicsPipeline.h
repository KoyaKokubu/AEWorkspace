#pragma once

#include "Utils/AREngineIncludes.h"

namespace AE {

	class Devices;

	struct PipelineConfigInfo {
		VkViewport viewport;
		VkRect2D scissor;
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
		void operator=(const GraphicsPipeline&) = delete;

		void createGraphicsPipeline(const char* vertFilePath, const char* fragFilePath, const PipelineConfigInfo& configInfo);
		static PipelineConfigInfo defaultPipelineConfig(uint32_t width, uint32_t height, PipelineConfigInfo& configInfo);
		void bind(VkCommandBuffer commandBuffer);

		VkPipeline& getGraphicsPipeline() { return m_graphicsPipeline; }

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
#include <stdlib.h>
#include <fstream>
#include <stdexcept>
#include <cassert>

#include "../Utils/AREngineDefines.h"
#include "../Devices.h"
#include "../Model.h"
#include "ComputePipeline.h"

namespace AE {

	ComputePipeline::ComputePipeline(
		Devices& devices,
		const char* systemFilePath
	) : m_devices{ devices }
	{
#ifdef COMPILE_SHADER_FILES
		compileShaderFiles(systemFilePath);
#endif
	}

	void ComputePipeline::compileShaderFiles(const char* systemFilePath) {
		system(systemFilePath);
	}

	std::vector<char> ComputePipeline::readFile(const char* filePath) {
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	void ComputePipeline::createComputePipeline(const char* compFilePath, VkPipelineLayout computePipelineLayout) {
		const std::vector<char> computeShaderCode = readFile(compFilePath);

		createShaderModule(computeShaderCode, &m_compShaderModule);

		VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
		computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		computeShaderStageInfo.module = m_compShaderModule;
		computeShaderStageInfo.pName = "main";

		// Create Compute Pipeline
		VkComputePipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.layout = computePipelineLayout;
		pipelineInfo.stage = computeShaderStageInfo;

		if (vkCreateComputePipelines(m_devices.getLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_computePipeline)
			!= VK_SUCCESS) {
			throw std::runtime_error("failed to create compute pipeline!");
		}

		vkDestroyShaderModule(m_devices.getLogicalDevice(), m_compShaderModule, nullptr);
	}

	void ComputePipeline::enableAlphaBlending(PipelineConfigInfo& configInfo) {
		// Color blending
		configInfo.colorBlendAttachment.blendEnable = VK_TRUE; // performance cost increase when VK_TRUE

		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		// src: the drawing fragment color
		// dst: the existing fragment color in the attachment
		// color.rgb = (src.a * src.rgb) + ((1 - src.a) * dst.rgb
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	}

	void ComputePipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(m_devices.getLogicalDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module");
		}
	}

	void ComputePipeline::bind(VkCommandBuffer commandBuffer) {
		// The second parameter specifies if the pipeline object is a graphics or compute pipeline. 
		// Let Vulkan know which operations to execute in the graphics pipeline and which attachment to use in the fragment shader.
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
	}

} // namespace AE
#pragma once

#include "../Utils/AREngineIncludes.h"
#include "../RenderSystem/GraphicsPipeline.h"

namespace AE {

	class Devices;

	class ComputePipeline {
	public:
		ComputePipeline(Devices& devices, const char* systemFilePath);
		~ComputePipeline() {}
		
		ComputePipeline(const ComputePipeline&) = delete;
		ComputePipeline& operator=(const ComputePipeline&) = delete;

		void createComputePipeline(const char* compFilePath, VkPipelineLayout computePipelineLayout);
		static void defaultPipelineConfig(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);
		void bind(VkCommandBuffer commandBuffer);

		const VkPipeline& getComputePipeline() const { return m_computePipeline; }

	private:
		static void compileShaderFiles(const char* systemFilePath);
		static std::vector<char> readFile(const char* filePath);
		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		Devices& m_devices;
		VkPipeline m_computePipeline;
		VkShaderModule m_vertShaderModule;
		VkShaderModule m_fragShaderModule;
		VkShaderModule m_compShaderModule;
	};

} // namespace AE
#pragma once

#include <iostream>
#include <optional>

#include "Utils/AREngineIncludes.h"

namespace AE {

	class WinApplication;
	class ValidationLayers;
	class SwapChain;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices {
		// It's not really possible to use a magic value to indicate the nonexistence of a queue family, since any value of uint32_t could in theory be a valid queue family index including 0.
		// std::optional is a wrapper that contains no value until you assign something to it. At any point you can query if it contains a value or not by calling its has_value() member function.
		std::optional<uint32_t> graphicsAndComputeFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsAndComputeFamily.has_value() && presentFamily.has_value();
		}
	};


	class Devices {
	public:
		Devices(AE::ValidationLayers& validLayers) : m_validLayers{ validLayers }
		{
		}

		// Not copyable or movable
		Devices(const Devices&) = delete;
		Devices& operator=(const Devices&) = delete;
		Devices(Devices&&) = delete;
		Devices& operator=(Devices&&) = delete;

		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& device, VkSurfaceKHR& surface);
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		
		void pickPhysicalDevice(VkInstance& vkInstance);
		void createLogicalDevice();
		void createSurface(VkInstance& vkInstance, WinApplication& winApp);
		void createImageWithInfo(
			const VkImageCreateInfo& imageInfo,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& imageMemory
		);
		void createCommandPool();
		void createBuffer(
			VkDeviceSize size,
			VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties,
			VkBuffer& buffer,
			VkDeviceMemory& bufferMemory
		);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

		VkPhysicalDevice& getPhysicalDevice() { return m_physicalDevice; }
		VkPhysicalDeviceProperties& getPhysicalDeviceProperties() { return m_properties; }
		VkDevice& getLogicalDevice() { return m_device; }
		VkSurfaceKHR& getSurface() { return m_surface; }
		VkCommandPool& getCommandPool() { return m_commandPool; }
		VkQueue& getGraphicsQueue() { return m_graphicsComputeQueue; }
		VkQueue& getGraphicsComandQueue() { return m_graphicsComputeQueue; }
		VkQueue& getPresentQueue() { return m_presentQueue; }
		VkSampleCountFlagBits getMSAAsamples() { return m_msaaSamples; }
		VkPhysicalDeviceFeatures& getDeviceFeatures() { return m_deviceFeatures; }

	private:
		bool isDeviceSuitable(VkPhysicalDevice device);
#ifdef RATE_SUITABLE_DEVICE
		int rateDeviceSuitability(VkPhysicalDevice device);
#endif
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		VkSampleCountFlagBits getMaxUsableSampleCount();

		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		ValidationLayers& m_validLayers;
		// physical device
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_properties;
		// logical device: not directly interact with instances.
		VkDevice m_device;
		VkQueue m_graphicsComputeQueue;
		VkQueue m_presentQueue;
		VkSurfaceKHR m_surface;
		VkCommandPool m_commandPool;
		VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT; // msaa: Multisample anti-aliasing
		VkPhysicalDeviceFeatures m_deviceFeatures;
	};
} // namespace AE
#pragma once

#include "Utils/AREngineIncludes.h"

namespace AE {

	class ValidationLayers;

	class VulkanInstance {
	public:
		VulkanInstance(const char* name, ValidationLayers& validLayers);
		void createInstance();
		VkInstance& getInstance() { return m_instance; }

	private:
		VkInstance m_instance;
		const char* m_appName;
		ValidationLayers& m_validLayers;
	};

} // namespace AE
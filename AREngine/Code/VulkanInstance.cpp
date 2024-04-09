#include "VulkanInstance.h"
#include "Utils/ValidationLayers.h"
#include "Utils/AREngineDefines.h"

namespace AE {

	VulkanInstance::VulkanInstance(const char* name, ValidationLayers& validLayers) : m_appName{ name }, m_validLayers{ validLayers }
	{
	}

	void VulkanInstance::createInstance() {
		if (m_validLayers.enableValidationLayers && !m_validLayers.checkValidationLayerSupport()) {
			throw std::runtime_error("Validation layers requested, but not available!");
		}

		// VkApplicationInfo
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = m_appName;
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// VkInstanceCreateInfo
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		// find device specific extensions
		std::vector<const char*> extensions = m_validLayers.getRequiredExtensions();
		// Attach device specific extensions
#ifdef WINDOWS_OS
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
#else
		std::vector<const char*> requiredExtensions;
		for (uint32_t i = 0; i < static_cast<uint32_t>(extensions.size()); i++) {
			requiredExtensions.emplace_back(extensions.data()[i]);
		}
		requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		if (m_validLayers.enableValidationLayers) {
			// VK_EXT_DEBUG_UTILS_EXTENSION_NAME macro is equal to the literal string "VK_EXT_debug_utils"
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
		createInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
#endif

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (m_validLayers.enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validLayers.m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validLayers.m_validationLayers.data();

			m_validLayers.populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

#ifdef ADD_DEBUG
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> vkExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensions.data());
		std::cout << "available extensions:\n";
		for (const VkExtensionProperties& extension : vkExtensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}
		std::cout << "\nGLFW required extensions:\n";
		for (int i = 0; i < extensions.size(); i++) {
			std::cout << '\t' << extensions[i] << '\n';
		}
#endif
		// create vulkan instance
		/*As you'll see, the general pattern that object creation function parameters in Vulkan follow is:
			- Pointer to struct with creation info
			- Pointer to custom allocator callbacks, always nullptr in this tutorial
			- Pointer to the variable that stores the handle to the new object*/
		if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create instance!");
		}
	}

} // namespace AE
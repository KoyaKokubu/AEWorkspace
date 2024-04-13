#pragma once

#include "../Utils/AREngineIncludes.h"

namespace AE {

	class WinApplication {
	public:
		WinApplication(int w, int h, const char* name);

		// resource aqcuisition is initialization
		WinApplication(const WinApplication&) = delete;
		WinApplication& operator=(const WinApplication&) = delete;

		void initWindow();
		bool shouldClose() { return glfwWindowShouldClose(m_pWin); }
		VkExtent2D getExtent() { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height) }; }
		bool wasWindowResized() { return m_framebufferResized; }
		void resetWindowResizedFlag() { m_framebufferResized = false; }

		GLFWwindow* getWindowPointer() const { return m_pWin; } ;

	private:
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

		int m_width;
		int m_height;
		bool m_framebufferResized = false;
		const char* m_winName;
		GLFWwindow* m_pWin;
	};

} // namespace AE
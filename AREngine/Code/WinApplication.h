#pragma once

#include "Utils/AREngineIncludes.h"

namespace AE {

	class WinApplication {
	public:
		WinApplication(int w, int h, const char* name);

		// resource aqcuisition is initialization
		WinApplication(const WinApplication&) = delete;
		WinApplication& operator=(const WinApplication&) = delete;

		void initWindow();
		bool shouldClose() { return glfwWindowShouldClose(m_pWin); }

		GLFWwindow* getWindowPointer();

	private:
		const int m_width;
		const int m_height;
		const char* m_winName;
		GLFWwindow* m_pWin;
	};

} // namespace AE
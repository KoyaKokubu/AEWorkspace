#include "WinApplication.h"

namespace AE {

	WinApplication::WinApplication(int w, int h, const char* name) : 
		m_width{w},
		m_height{h},
		m_winName{name}
	{
	}

	void WinApplication::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_pWin = glfwCreateWindow(m_width, m_height, m_winName, nullptr, nullptr);
	}

	GLFWwindow* WinApplication::getWindowPointer() {
		return m_pWin;
	}

} // namespace AE
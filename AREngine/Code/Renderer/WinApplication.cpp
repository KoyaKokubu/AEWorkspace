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
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_pWin = glfwCreateWindow(m_width, m_height, m_winName, nullptr, nullptr);
		glfwSetWindowUserPointer(m_pWin, this);
		glfwSetFramebufferSizeCallback(m_pWin, framebufferResizeCallback);
	}

	void WinApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		WinApplication* winApp = reinterpret_cast<WinApplication*>(glfwGetWindowUserPointer(window));
		winApp->m_framebufferResized = true;
		winApp->m_width = width;
		winApp->m_height = height;
	}

} // namespace AE
//#include <glm/vec4.hpp>
//#include <glm/mat4x4.hpp>
#include "Application.h"

#include <opencv2/opencv.hpp>

int main() {
	AE::Application app{};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

//int main()
//{
//	// Vulkan part
//	glfwInit();
//	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);
//	uint32_t extensionCount = 0;
//	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//	std::cout << extensionCount << " extensions supported" << std::endl;
//	glm::mat4 matrix;
//	glm::vec4 vec;
//	auto test = matrix * vec;
//	while (!glfwWindowShouldClose(window)) {
//		glfwPollEvents();
//	}
//	glfwDestroyWindow(window);
//	glfwTerminate();
//
//	// OpenCV part
//	const char* windowName = "OpenCV window";
//
//	cv::Mat img = cv::imread("C:\\Users\\meina\\Pictures\\ReadyPlayerMe-Avatar.jpeg");
//	if (img.data == NULL) {
//		printf("file read error");
//	}
//	else {
//		cv::imshow(windowName, img);
//	}
//
//	cv::waitKey(0);
//
//	return 0;
//}
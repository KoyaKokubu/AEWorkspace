#include <fstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "RGBDvision.h"

namespace AE {

	void RGBDvision::setCameraExternalParameters() {
		std::ifstream fin("3DVision/RGBD/pose.txt");
		assert(fin && "Please run the program in the directory that has pose.txt");
        std::string png = ".png";
        std::string pgm = ".pgm";
        for (int i = 0; i < 5; i++) {
            int index = i + 1;
            std::string colorPath = "3DVision/RGBD/color/" + std::to_string(index) + png;
            std::string depthPath = "3DVision/RGBD/depth/" + std::to_string(index) + pgm;
            //m_colorImgs.emplace_back(cv::imread(colorPath, cv::IMREAD_UNCHANGED));
            m_colorImgs.emplace_back(cv::imread(colorPath));
            if (m_colorImgs[i].data == NULL) {
                printf("file read error");
            }
            //cv::cvtColor(m_colorImgs[i], m_colorImgs[i], cv::COLOR_BGR2RGBA);
            m_depthImgs.emplace_back(cv::imread(depthPath, -1));
            if (m_depthImgs[i].data == NULL) {
                printf("file read error");
            }

            float data[7] = { 0 };
            for (auto& d : data) {
                fin >> d;
            }

            // Position
            glm::vec3 cameraPosition = { data[0], data[1], data[2] };
            // Rotation (Quaternion -> Mat4)
            glm::quat cameraQuaternion{ data[6], data[3], data[4], data[5] };
            glm::mat4 cameraRotationMat = glm::toMat4(cameraQuaternion);

            m_cameraPos.emplace_back(cameraPosition);
            m_cameraRotation.emplace_back(cameraRotationMat);
            int debug = 0;
        }
	}

	void RGBDvision::generatePointCloud() {
        float cx = 325.5;
        float cy = 253.5;
        float fx = 518.0;
        float fy = 519.0;
        float depthScale = 1000.0;

        int imageNum = 5;
        std::vector<std::vector<glm::vec4>> pointCloud_position(imageNum);
        std::vector<std::vector<glm::vec4>> pointCloud_color(imageNum);
        std::vector<int> particleNum(imageNum);

        for (int i = 0; i < imageNum; i++) {
            std::cout << "Converting RGBD images " << i + 1 << std::endl;
            cv::Mat color = m_colorImgs[i];
            cv::Mat depth = m_depthImgs[i];

            const char* windowName = "OpenCV window";
            cv::imshow(windowName, color);
            cv::waitKey(0);

            pointCloud_position[i].reserve(color.rows * color.cols);
            pointCloud_color[i].reserve(color.rows * color.cols);

            // set camera pose
            setViewPose(m_cameraPos[i], m_cameraRotation[i]);

            for (int v = 0; v < color.rows; v++) {
                for (int u = 0; u < color.cols; u++) {
                    unsigned int d = depth.ptr<unsigned short>(v)[u];
                    if (d == 0) {
                        continue;
                    }

                    glm::vec4 point{ 1.f };
                    point[2] = float(d) / depthScale;
                    point[0] = (u - cx) * point[2] / fx;
                    point[1] = (v - cy) * point[2] / fy;
                    glm::vec4 pointWorld = m_inverseViewMatrix * point;

                    glm::vec4 pointColor{ 1.f };
                    pointColor[0] = color.data[v * color.step + u * color.channels() + 2] / 255.f; // red
                    pointColor[1] = color.data[v * color.step + u * color.channels() + 1] / 255.f; // green
                    pointColor[2] = color.data[v * color.step + u * color.channels()] / 255.f;     // blue

                    pointCloud_position[i].emplace_back(pointWorld);
                    pointCloud_color[i].emplace_back(pointColor);
                }
            }
            particleNum[i] = pointCloud_position[i].size();
        }
        m_particleSystem.setPointCloud(imageNum, particleNum, pointCloud_position, pointCloud_color);
    }

    void RGBDvision::setViewPose(glm::vec3 position, glm::mat4 rotationMat) {
        const glm::vec3 u{ rotationMat[0] };
        const glm::vec3 v{ rotationMat[1] };
        const glm::vec3 w{ rotationMat[2] };
        m_inverseViewMatrix = glm::mat4{ 1.f };
        m_inverseViewMatrix[0][0] = u.x;
        m_inverseViewMatrix[0][1] = u.y;
        m_inverseViewMatrix[0][2] = u.z;
        m_inverseViewMatrix[1][0] = v.x;
        m_inverseViewMatrix[1][1] = v.y;
        m_inverseViewMatrix[1][2] = v.z;
        m_inverseViewMatrix[2][0] = w.x;
        m_inverseViewMatrix[2][1] = w.y;
        m_inverseViewMatrix[2][2] = w.z;
        m_inverseViewMatrix[3][0] = position.x;
        m_inverseViewMatrix[3][1] = position.y;
        m_inverseViewMatrix[3][2] = position.z;
    }


} // namespace AE
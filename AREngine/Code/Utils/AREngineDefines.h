#pragma once

#define APPLICATION_NAME "Vulkan Application"

#define WINDOWS_OS
//#define ADD_DEBUG

//// Pick either one.
//#define SPECIFIC_SUITABLE_DEVICE
//#define RATE_SUITABLE_DEVICE

#define COMPILE_SHADER_FILES
#define SIMPLE_SHADER_COMPILER_PATH "Shaders\\compile_simple.bat"
#define POINT_SHADER_COMPILER_PATH "Shaders\\compile_point.bat"
#define SIMPLE_VERT_SHADER_PATH "Shaders\\simple_shader.vert.spv"
#define SIMPLE_FRAG_SHADER_PATH "Shaders\\simple_shader.frag.spv"
#define POINT_LIGHT_VERT_SHADER_PATH "Shaders\\point_light.vert.spv"
#define POINT_LIGHT_FRAG_SHADER_PATH "Shaders\\point_light.frag.spv"

// max number of frames in flight
#define MAX_FRAMES_IN_FLIGHT 2

#define TINYOBJLOADER_IMPLEMENTATION

#define MAX_LIGHTS 10 // Don't forget to update this in shaders too unless implementing the specialization constants
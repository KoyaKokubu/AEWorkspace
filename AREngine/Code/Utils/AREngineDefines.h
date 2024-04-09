#pragma once

#define WINDOWS_OS
//#define ADD_DEBUG

//// Pick either one.
//#define SPECIFIC_SUITABLE_DEVICE
//#define RATE_SUITABLE_DEVICE

//#define COMPILE_SHADER_FILES
#define SYSTEM_FILE_PATH "Shaders\\compile.bat"
#define VERT_SHADER_PATH "Shaders\\shader.vert.spv"
#define FRAG_SHADER_PATH "Shaders\\shader.frag.spv"

// max number of frames in flight
#define MAX_FRAMES_IN_FLIGHT 2
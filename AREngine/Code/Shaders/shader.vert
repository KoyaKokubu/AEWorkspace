#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

// layout(location = 0) out vec3 fragColor;

// Must match the order specified in SimplePushConstantData
// Can be used per shader  entry point
layout(push_constant) uniform Push {
	vec2 offset;
	vec3 color;
} push;

void main() {
	gl_Position = vec4(inPosition + push.offset, 0.0, 1.0);
	// fragColor = inColor;
}
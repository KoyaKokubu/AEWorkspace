#version 450

layout (location = 0) in vec4 fragColor;
layout (location = 1) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

struct PointLight {
	vec4 position; // ignore w
	vec4 color; // w is intensity
};

layout (set = 0, binding = 0) uniform GlobalUbo {
	mat4 projection;
	mat4 view;
	mat4 invView;
	vec4 ambientLightColor; // w is intensity
	PointLight pointLights[10];
	int numLights;
} ubo;

const float M_PI = 3.1415926538;

void main() {
	// outColor = fragColor;
	
	float distance = sqrt(dot(fragOffset, fragOffset));
	if (distance >= 1) {
		discard;
	}

	float cosDis = 0.5 * (cos(distance * M_PI) + 1.0);
	outColor = vec4(fragColor.xyz + cosDis, cosDis);
}
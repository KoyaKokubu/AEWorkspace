#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec3 fragPosWorld;
layout (location = 2) out vec3 fragNormalWorld;
layout (location = 3) out vec2 fragUV;

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

// Must match the order specified in SimplePushConstantData
// Can be used per shader  entry point
layout (push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	vec4 positionWorld = push.modelMatrix * vec4(inPosition, 1.0);
	gl_Position = ubo.projection * ubo.view * positionWorld;
	
	// Normal Transform : https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
	fragNormalWorld = normalize(mat3(push.normalMatrix) * inNormal);
	fragPosWorld = positionWorld.xyz;
	fragColor = inColor;
	fragUV = inUV;
}
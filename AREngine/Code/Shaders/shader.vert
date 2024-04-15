#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projectionViewMatrix;
	vec3 directionToLight;
} ubo;

// Must match the order specified in SimplePushConstantData
// Can be used per shader  entry point
layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

const float AMBIENT = 0.02;

void main() {
	gl_Position = ubo.projectionViewMatrix * push.modelMatrix * vec4(inPosition, 1.0);
	
	//// Normal Transform : https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
	//// vec3 normalWorldSpace = normalize(mat3(push.modelMatrix) * inNormal);
	//mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
	//vec3 normalWorldSpace = normalize(normalMatrix * inNormal);

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * inNormal);
	
	float lightIntensity = AMBIENT + max(dot(normalWorldSpace, ubo.directionToLight), 0);
	fragColor = lightIntensity * inColor;
}
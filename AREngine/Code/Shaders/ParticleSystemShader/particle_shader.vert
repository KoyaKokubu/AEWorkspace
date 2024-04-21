#version 450

layout (location = 0) in vec3 inPosition;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragOffset;
//layout (location = 1) out float fragOffset;

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

struct Particle {
	vec4 position;
	vec4 color;
	vec4 velocity;
};

layout (std140, set = 0, binding = 3) readonly buffer ParticleSSBOin {
	Particle particlesIn[ ];
} PointCloud;

const float RADIUS = 0.01;

void main() {	
	//gl_PointSize = 1.0;
	//gl_Position = ubo.projection * ubo.view * vec4(PointCloud.particlesIn[gl_InstanceIndex].position.xyz, 1.0);
	float distance = dot(PointCloud.particlesIn[gl_InstanceIndex].position.xyz, PointCloud.particlesIn[gl_InstanceIndex].position.xyz);
	float colorX = PointCloud.particlesIn[gl_InstanceIndex].position.x / distance;
	float colorY = PointCloud.particlesIn[gl_InstanceIndex].position.y / distance;
	
	fragColor = vec4(colorX, colorY, 1.0, 1.0); // PointCloud.particlesIn[gl_InstanceIndex].color;
	
	fragOffset = inPosition.xy;
	vec3 cameraRightWorld = { ubo.view[0][0], ubo.view[1][0], ubo.view[2][0] };
	vec3 cameraUpWorld = { ubo.view[0][1], ubo.view[1][1], ubo.view[2][1] };
	
	vec3 positionWorld = PointCloud.particlesIn[gl_InstanceIndex].position.xyz
		+ RADIUS * inPosition.x * cameraRightWorld
		+ RADIUS * inPosition.y * cameraUpWorld;
	// vec3 positionWorld = inPosition.xyz; //PointCloud.particlesIn[gl_InstanceIndex].position.xyz;
	gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}
#version 450
#extension GL_ARB_separate_shader_objects : enable

// this defines the variable received from the Vertex Shader
// the locations must match the one of its out variables
layout(location = 0) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;

// This defines the color computed by this shader. Generally is always location 0.
layout(location = 0) out vec4 outColor;




layout(set = 0, binding = 1) uniform sampler2D tex;



void main() {
	vec3 Norm = normalize(fragNormal);
	vec3 Albedo = texture(tex, fragUV).rgb;


	// Indirect illumination simulation
	// A special type of non-uniform ambient color, invented for this course
	const vec3 cxp = vec3(1.0,0.5,0.5) * 0.2;
	const vec3 cxn = vec3(0.9,0.6,0.4) * 0.2;
	const vec3 cyp = vec3(0.3,1.0,1.0) * 0.2;
	const vec3 cyn = vec3(0.5,0.5,0.5) * 0.2;
	const vec3 czp = vec3(0.8,0.2,0.4) * 0.2;
	const vec3 czn = vec3(0.3,0.6,0.7) * 0.2;
	
	vec3 Ambient =((Norm.x > 0 ? cxp : cxn) * (Norm.x * Norm.x) +
				   (Norm.y > 0 ? cyp : cyn) * (Norm.y * Norm.y) +
				   (Norm.z > 0 ? czp : czn) * (Norm.z * Norm.z)) * Albedo;



	outColor = vec4(Ambient, 1.0f);
}
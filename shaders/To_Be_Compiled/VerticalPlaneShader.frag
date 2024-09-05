#version 450
#extension GL_ARB_separate_shader_objects : enable

// this defines the variable received from the Vertex Shader
// the locations must match the one of its out variables
layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragNormal;

// This defines the color computed by this shader. Generally is always location 0.
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	vec4 color;
} ubo;

layout(binding = 2) uniform GlobalUniformBufferObject {
	vec3 lightDir;
	vec3 lightPos;
	vec4 lightColor;
	vec3 eyePos;
	vec4 eyeDir;
} gubo;
	

layout(set = 0, binding = 1) uniform sampler2D tex;

vec3 BRDF(vec3 Albedo, vec3 Norm, vec3 EyeDir, vec3 LD) {
// Compute the BRDF, with a given color <Albedo>, in a given position characterized bu a given normal vector <Norm>,
// for a light direct according to <LD>, and viewed from a direction <EyeDir>
	vec3 Diffuse;
	vec3 Specular;
	Diffuse = Albedo * max(dot(Norm, LD),0.0f);
	Specular = vec3(pow(max(dot(EyeDir, -reflect(LD, Norm)),0.0f), 160.0f));
	
	return Diffuse + Specular;
}



void main() {
	vec3 Norm = normalize(fragNormal);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	vec3 Albedo = texture(tex, fragUV).rgb;

	vec3 LD = normalize(gubo.lightDir - fragPos);
	float distance = length(gubo.lightPos - fragPos);
    float attenuation = 1.0 / (1.0 + pow(distance, 2.0));
	vec3 LC = gubo.lightColor.rgb * attenuation * gubo.lightColor.a;

	vec3 RendEqSol = vec3(0);

	RendEqSol = BRDF(Albedo, Norm, EyeDir, LD) * LC;

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

	RendEqSol += Ambient;

	outColor = vec4(RendEqSol, 1.0f);
}
#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	vec4 lightDir[3];
	vec4 lightPos;
	vec4 lightColor[3];
	vec4 eyePos;
} gubo;


layout(set = 1, binding = 1) uniform sampler2D tex;


//For PointLight regulations
float constant = 1.0;
float linear = 0.0001;
float quadratic = 0.0001;
// PointLight


vec3 BRDF(vec3 Albedo, vec3 Norm, vec3 EyeDir, vec3 LD){
    vec3 Diffuse;
    vec3 Specular;
    Diffuse = Albedo * max(dot(Norm, LD), 0.0f);
    Specular = vec3(pow(max(dot(EyeDir, -reflect(LD, Norm)), 0.0f), 160.0f));

    return Diffuse + 0.6 * Specular;
}


void main() {
    vec3 lightColor[3];
    vec3 lightDir[3];

    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos.xyz - fragPos);
    vec3 Albedo = texture(tex, fragUV).rgb;

    vec3 RendEqSol = vec3(0.0f);

    //Point Light
    lightDir[0] = normalize(gubo.lightDir[0].xyz);
    float distance = length(gubo.lightPos.xyz - fragPos);
    
    // Modifica dell'attenuazione per aumentare il raggio d'azione
    float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);

    lightColor[0] = gubo.lightColor[0].rgb * attenuation * gubo.lightColor[0].a;

    RendEqSol += BRDF(Albedo, Norm, EyeDir, lightDir[0]) * lightColor[0].xyz;

    vec3 Ambient = texture(tex, fragUV).rgb * 0.025f;

    RendEqSol += Ambient;
    
    outColor = vec4(RendEqSol, 1.0f);
}

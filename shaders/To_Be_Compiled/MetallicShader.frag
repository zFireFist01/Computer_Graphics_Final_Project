#version 450
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.14159

// this defines the variable received from the Vertex Shader
// the locations must match the one of its out variables
layout(location = 0) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 1) in vec3 fragNorm;

// This defines the color computed by this shader. Generally is always location 0.
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	vec4 lightDir[3];
	vec4 lightPos;
	vec4 lightColor[3];
	vec4 eyePos;
} gubo;


layout(set = 1, binding = 1) uniform sampler2D tex;


//For PointLight regulations
float quadratic = 0.01;
// PointLight

//For Ambient Light
float ambientIntensity = 0.8f;

//For Cook-Torrance
float roughness = 0.1; 
float k = 0.6; 
float F0 = 0.91; 

float ggx(vec3 n, vec3 v){
    float dotNV = max(dot(n,v), 0.0);
    float k = (roughness * roughness) / 2.0;
    return dotNV / (dotNV * (1.0 - k) + k);
}


// Cook-Torrance
vec3 CookTorrance(vec3 lightColor, vec3 Norm, vec3 EyeDir, vec3 LD) {
    vec3 Diffuse;
    vec3 Specular;

    // Lambert
    Diffuse = lightColor * max(dot(Norm, LD), 0.0f);

    // Cook-Torrance
    vec3 halfVec = normalize(EyeDir + LD);
    //float NdotH = max(dot(Norm, halfVec), 0.0);
    float NdotV = max(dot(Norm, EyeDir), 0.0);
    //float NdotL = max(dot(Norm, LD), 0.0);
    float VdotH = max(dot(EyeDir, halfVec), 0.0);

    float D = (roughness * roughness) / (PI * pow( pow( dot(Norm, halfVec), 2) * (roughness * roughness - 1) + 1, 2));
    float G = ggx(Norm, EyeDir) * ggx(Norm, LD);
    float F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

    Specular = lightColor * D * G * F / (4.0 * NdotV);

    return k*Diffuse + (1-k)*Specular;
}


void main() {
    vec3 lightColor;
    vec3 lightDir;

    vec3 Norm = fragNorm;
    vec3 EyeDir = normalize(gubo.eyePos.xyz - fragPos);
    vec4 Albedo = texture(tex, fragUV);

    vec3 RendEqSol = vec3(0.0f);

    //Point Light
    lightDir = normalize(gubo.lightDir[0].xyz);
    float distance = length(gubo.lightPos.xyz - fragPos);
    
    // Modifica dell'attenuazione per aumentare il raggio d'azione
    float attenuation = 1.0 / (quadratic * distance * distance);

    lightColor = gubo.lightColor[0].rgb * attenuation;

    // Call CookTorrance function in main()
    RendEqSol += CookTorrance(lightColor, Norm, EyeDir, lightDir);

    vec3 Ambient = vec3(1, 1, 1) * ambientIntensity;

    outColor = Albedo * vec4((RendEqSol + Ambient) * vec3(1, 1, 1), 1.0);
}
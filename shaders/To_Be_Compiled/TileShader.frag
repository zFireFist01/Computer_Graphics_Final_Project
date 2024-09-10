#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;



void main() {
    vec3 Norm = fragNorm;
    vec2 UV = fragUV;
    vec3 Pos = fragPos;
    outColor = vec4(vec3(1.0f,0.0f,1.0f), 1.0f);
}
#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
    vec4 color;
} ubo;

void main() {
    fragUV = inUV;
    fragNormal = (ubo.nMat * vec4(inNormal, 0.0)).xyz;
    fragColor = ubo.color.rgb;
    gl_Position = ubo.mvpMat * vec4(inPos, 1.0);
}

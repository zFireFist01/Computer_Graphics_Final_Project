#version 450

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 color = texture(texSampler, fragUV).rgb;
    outColor = vec4(color, 1.0);
}

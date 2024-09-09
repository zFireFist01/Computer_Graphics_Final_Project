#version 450
#extension GL_ARB_separate_shader_objects : enable

// The attributes associated with each vertex.
layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inUV;
layout(location = 1) in vec3 inNormal;

// Outputs to the fragment shader

layout(location = 0) out vec3 fragPos;
layout(location = 2) out vec2 fragUV;
layout(location = 1) out vec3 fragNormal;

// Uniform buffer object containing transformation matrices and color
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;
    mat4 mMat;
    mat4 nMat;
} ubo;

void main() {
    // Calculate the position in clip space
    gl_Position = ubo.mvpMat * vec4(inPosition, 1.0);

    // Pass the UV coordinates directly to the fragment shader
    fragUV = inUV;

    // Calculate the position in world space and pass it to the fragment shader
    fragPos = (ubo.mMat * vec4(inPosition, 1.0)).xyz;

    // Transform the normal vector with the normal matrix and pass it to the fragment shader
    fragNormal = normalize(vec3(ubo.nMat * vec4(inNormal, 0.0)));
}

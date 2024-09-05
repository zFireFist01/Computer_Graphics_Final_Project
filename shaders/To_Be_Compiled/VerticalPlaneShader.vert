#version 450
#extension GL_ARB_separate_shader_objects : enable


// Uniform buffer object che contiene le matrici e i colori per i piani
layout(binding = 0) uniform UniformBufferObject {
    mat4 mvpMat;  // Matrici MVP (Model-View-Projection) per ciascun piano
    mat4 mMat;    // Matrici Model per ciascun piano
    mat4 nMat;    // Matrici Normali per ciascun piano
    vec4 color;   // Colori per ciascun piano
} ubo;

// Input dal vertex buffer
layout(location = 0) in vec3 inPos;    // Posizione del vertice
layout(location = 1) in vec2 inUV;     // Coordinate UV del vertice
layout(location = 2) in vec3 inNormal; // Normale del vertice

// Output verso il fragment shader
layout(location = 0) out vec3 fragPos;      // Posizione del vertice nel mondo
layout(location = 1) out vec2 fragUV;       // Coordinate UV per la texture
layout(location = 2) out vec3 fragNormal;   // Normale del vertice interpolata




void main() {
    // Calculate the position in clip space
    gl_Position = ubo.mvpMat * vec4(inPos, 1.0);

    // Pass the UV coordinates directly to the fragment shader
    fragUV = inUV;

    // Calculate the position in world space and pass it to the fragment shader
    fragPos = (ubo.mMat * vec4(inPos, 1.0)).xyz;

    // Transform the normal vector with the normal matrix and pass it to the fragment shader
    fragNormal = normalize(vec3(ubo.nMat * vec4(inNormal, 0.0)));
}

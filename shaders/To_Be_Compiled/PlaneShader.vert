#version 450

const int NPLANE = 2; // Numero di piani

// Uniform buffer object che contiene le matrici e i colori per i piani
layout(binding = 0) uniform PlaneUniformBufferObject {
    mat4 mvpMat[NPLANE];  // Matrici MVP (Model-View-Projection) per ciascun piano
    mat4 mMat[NPLANE];    // Matrici Model per ciascun piano
    mat4 nMat[NPLANE];    // Matrici Normali per ciascun piano
    vec4 color[NPLANE];   // Colori per ciascun piano
} ubo;

// Input dal vertex buffer
layout(location = 0) in vec3 inPos;    // Posizione del vertice
layout(location = 1) in vec2 inUV;     // Coordinate UV del vertice
layout(location = 2) in vec3 inNormal; // Normale del vertice

// Output verso il fragment shader
layout(location = 0) out vec2 fragUV;       // Coordinate UV per la texture
layout(location = 1) out vec3 fragNormal;   // Normale del vertice interpolata
layout(location = 2) out vec4 fragColor;    // Colore del vertice interpolato
layout(location = 3) out vec3 fragPos;      // Posizione del vertice nel mondo

void main() {
    // Usa gl_InstanceIndex per selezionare l'indice corretto nell'array di uniform
    int index = gl_InstanceIndex;

    // Passa le coordinate UV al fragment shader
    fragUV = inUV;

    // Trasforma la normale usando la matrice normale appropriata e normalizzala
    fragNormal = normalize(mat3(ubo.nMat[index]) * inNormal);

    // Passa il colore del piano al fragment shader
    fragColor = ubo.color[index];

    // Calcola la posizione del vertice nello spazio mondo
    fragPos = vec3(ubo.mMat[index] * vec4(inPos, 1.0));

    // Calcola la posizione finale del vertice nel clip space usando la matrice MVP
    gl_Position = ubo.mvpMat[index] * vec4(inPos, 1.0);
}
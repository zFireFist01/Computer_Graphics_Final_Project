#version 450

// Uniform per la texture
layout(binding = 1) uniform sampler2D texSampler;

// Input dal vertex shader
layout(location = 0) in vec2 fragUV;        // Coordinate UV interpolate
layout(location = 1) in vec3 fragNormal;    // Normale interpolata
layout(location = 2) in vec4 fragColor;     // Colore interpolato dal vertex shader
layout(location = 3) in vec3 fragPos;       // Posizione del frammento nel mondo

// Output del colore finale
layout(location = 0) out vec4 outColor;

// Uniform per l'illuminazione globale e la posizione dell'occhio
layout(binding = 2) uniform GlobalUniformBufferObject {
    vec3 lightDir;   // Direzione della luce
    vec4 lightColor; // Colore della luce
    vec3 eyePos;     // Posizione della camera/occhio
    vec4 eyeDir;     // Direzione dell'occhio (non utilizzato qui, ma potrebbe servire)
} globalUbo;

void main() {
    // Normalizza la normale interpolata per l'illuminazione
    vec3 norm = normalize(fragNormal);

    // Normalizza la direzione della luce
    vec3 lightDir = normalize(globalUbo.lightDir);

    // Calcola l'illuminazione diffusa con il prodotto scalare tra normale e direzione della luce
    float diff = max(dot(norm, lightDir), 0.0);

    // Campiona il colore della texture usando le coordinate UV interpolate
    vec4 texColor = texture(texSampler, fragUV);

    // Calcola il colore finale del frammento combinando colore del piano, texture e illuminazione
    outColor = fragColor * texColor * diff * globalUbo.lightColor;

    // Aggiunge un minimo di luminosità per evitare che i frammenti diventino completamente neri
    // Aumentato per illuminare maggiormente la scena
    outColor.rgb += 0.2 * texColor.rgb;

    // Aumenta la componente di luce ambientale
    outColor.rgb = mix(outColor.rgb, fragColor.rgb * texColor.rgb, 0.3);
}

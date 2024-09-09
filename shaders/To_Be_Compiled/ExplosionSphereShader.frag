#version 450
#extension GL_ARB_separate_shader_objects : enable

// this defines the variable received from the Vertex Shader
// the locations must match the one of its out variables
layout(location = 0) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;
layout(location = 1) in vec3 fragNormal;

// This defines the color computed by this shader. Generally is always location 0.
layout(location = 0) out vec4 outColor;




layout(set = 0, binding = 1) uniform sampler2D tex;



void main() {
	// Intensity of ambient light
    float ambientIntensity = 1.5;
    vec3 ambientLightColor =vec3(1.0, 1.0, 1.0); // White light

    // Texture color
    vec4 texColor = texture(tex, fragUV);

    // Apply ambient light
    vec3 ambient = ambientIntensity * ambientLightColor;

    // Final color output combining texture color and ambient lighting
    outColor = vec4(texColor.rgb * ambient, texColor.a);
}

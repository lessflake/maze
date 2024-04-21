#version 330 core

// Create fade effect for when victory is achieved, if fade float
// passed in as uniform is not 0 (1 = fade in, 2 = fade out)

in vec2 TexCoords;
out vec4 color;

uniform sampler2D screenTexture;
uniform float time;
uniform float fade;

void main()
{
    color = texture(screenTexture, TexCoords);
    if (fade >= 1.5) {
        float modT = 1.0 - mod(time, 300)/300.0;
        color = modT * color;
    } else if (fade >= 0.5) {
        float modT = mod(time, 150)/150.0;
        color = modT * color;
    }
}

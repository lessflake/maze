#version 450 core

/* Creates drop shadow effect on minimap by sampling nearby TexCoords */

in vec2 TexCoord;
out vec4 color;

uniform sampler2D ourTexture;
uniform float shadowSize;

float border() {
    if (TexCoord.x + shadowSize > 1.0 || TexCoord.y + shadowSize > 1.0)
        return 0.0;
    return texture(ourTexture, TexCoord + shadowSize).w;
}

void main()
{
    color = texture(ourTexture, TexCoord);
    if (color.w == 0)
        color.w = border();
}

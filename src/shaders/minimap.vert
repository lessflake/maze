#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;

void main()
{
    // Z position is negative to ensure it's always rendered in front
    gl_Position = vec4(position.xy, -1.0, 1.0);
    TexCoord = texCoord;
}

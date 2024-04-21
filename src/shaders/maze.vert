#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 nNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 normal;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    FragPos = vec3(model * vec4(position, 1.0));
    TexCoord = texCoord;
    // Normal matrix to ensure normals are uniform
    nNormal = mat3(transpose(inverse(model))) * normal;
}

#version 410 core

layout(location = 0) in vec3 a_position;

out vec4 v_position;

uniform float scale;

void main()
{
    v_position = vec4(a_position * scale, 1.0);
    gl_Position = v_position;
}
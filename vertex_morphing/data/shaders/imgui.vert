#version 410 core
#if defined(GL_ES)
precision highp float;
#endif

uniform mat3 ProjMtx;

layout(location = 0) in vec2 Position;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec4 Color;

out vec2 Frag_UV;
out vec4 Frag_Color;

void main()
{
    Frag_UV = UV;
    Frag_Color = Color;

    vec3 newPos = ProjMtx * vec3(Position, 1.0);
    gl_Position = vec4(newPos.xy, 0.0, 1.0);
}

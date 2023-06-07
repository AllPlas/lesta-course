#version 410 core
#if defined(GL_ES)
precision highp float;
#endif

uniform sampler2D Texture;

in vec2 Frag_UV;
in vec4 Frag_Color;

out vec4 fragColor;

void main()
{
    fragColor = Frag_Color * texture(Texture, Frag_UV);
}

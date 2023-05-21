#version 410 core

#ifdef GL_ES
precision highp float;
#endif

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D texSampler;

void main()
{
    fragColor = texture(texSampler, texCoord);
}
#ifdef GL_ES
precision highp float;
#endif

in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D texSampler;

void main()
{
    vec4 color = texture(texSampler, texCoord);
    if (color.a == 0.0) discard;

    fragColor = color;
}
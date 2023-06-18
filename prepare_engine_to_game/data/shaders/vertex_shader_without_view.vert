layout(location = 0) in vec2 vertPosition;
layout(location = 1) in vec2 vertTexCoord;
layout(location = 2) in vec4 vertColor;

out vec2 texCoord;

uniform mat3 matrix;

void main()
{
    texCoord = vertTexCoord;
    vec3 pos = matrix * vec3(vertPosition, 1.0);
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}
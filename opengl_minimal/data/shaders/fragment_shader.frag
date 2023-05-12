#version 410 core

precision mediump float;

in vec4 v_position;
out vec4 frag_color;

uniform float time;
uniform vec2 center;
uniform float spiral_speed;
uniform float spiral_density;


void main()
{
    // Вычисляем радиус спирали
    float radius = length(v_position.xy);

    // Вычисляем угол спирали с учетом времени и скорости вращения
    float angle = spiral_speed * time + radius * spiral_density * 100;

    // Вычисляем новые координаты спирали
    vec2 spiral = vec2(cos(angle), sin(angle)) * radius;
    if (v_position.z <= -0.5) {
        frag_color = vec4(mix(vec3(spiral * 0.1, 0.0), vec3(spiral, 1.0), abs(sin(time))), 1.0);
    }
    else {
        frag_color = vec4(1.0);
    }
}
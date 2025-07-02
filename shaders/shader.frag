#version 450

layout(location = 0) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

vec3 hsvToRgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    // Вычисляем цвет на основе позиции и времени
    float hue = mod(fragPos.z * 100.0 + fragPos.x * 200.0, 360.0);
    outColor = vec4(hsvToRgb(vec3(hue, 1.0, 1.0)), 1.0);
}
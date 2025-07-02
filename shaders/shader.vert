#version 450

layout(location = 0) in vec3 inPosition;  // Только позиция

layout(location = 0) out vec3 fragPos;    // Передаем позицию в фрагментный шейдер

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;  // Время для анимации
} ubo;

void main() {
    // Вычисляем высоту на основе времени и позиции
    float height = sin(ubo.time + (inPosition.x - inPosition.y) * 10.0) * 0.1;
    vec3 animatedPos = vec3(inPosition.x, inPosition.y, height);

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(animatedPos, 1.0);
    fragPos = animatedPos;  // Передаем анимированную позицию
}
#version 450

layout(location = 0) out vec2 fragPos;    // Передаем позицию в фрагментный шейдер

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
	uint size;  // Время для анимации
} ubo;

void main() {
    // Вычисляем координаты x и y из индекса вершины
    uint index = gl_VertexIndex;
    uint x = index / ubo.size;  // Целочисленное деление
    uint y = index % ubo.size;  // Остаток от деления

    // Преобразуем в float и нормализуем в диапазон [0..1]
    float cord_x = float(x) / float(ubo.size - 1);
    float cord_y = float(y) / float(ubo.size - 1);

    // Высота с анимацией
    float height = sin(ubo.time + (cord_x - cord_y) * 10.0) * 0.1;
    vec3 animatedPos = vec3(cord_x, cord_y, height);

    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(animatedPos, 1.0);
    fragPos = vec2(cord_x, cord_y);
}
#version 450

layout(location = 0) in vec2 fragTexCoord;  // Получаем текстурные координаты
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;  // Сэмплер для текстуры

void main() {
    outColor = texture(texSampler, fragTexCoord);
}
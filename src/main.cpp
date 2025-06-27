#include "vk.hpp"


#include <iostream>

void vkInit();

int main(int argc, char* argv[]) {

    // Инициализация GLFW
    glfwInit();

    // Проверка доступности Vulkan
    if (glfwVulkanSupported()) {
        // объект класса-обертки Vulkan API
        Vulkan vulkan;

        // Отключим создание контекста
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        // Отключим возможность изменения размеров окна
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        // Создание окна
        GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

        // Инициализация Vulkan API
        vulkan.init(window);

        // Жизненный цикл
        while(!glfwWindowShouldClose(window)) {
            // Обработка событий
            glfwPollEvents();
            vulkan.renderFrame();
        }

        // Уничтожение окна
        glfwDestroyWindow(window);

        // Завершение работы с Vulkan
        vulkan.destroy();
    } else
        std::cout << "There is no Vulkan Supported\n";

    // Завершение работы с GLFW
    glfwTerminate();

    return 0;
}
#include "vk.hpp"
#include <chrono>

#include <iostream>

void vkInit();

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        std::cout << "W pressed" << std::endl;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        std::cout << "A pressed" << std::endl;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        std::cout << "S pressed" << std::endl;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        std::cout << "D pressed" << std::endl;

    // Добавим выход по Escape для удобства
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

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

            // Переменные для FPS
        auto lastTime = std::chrono::high_resolution_clock::now();
		auto lastFrameTime = lastTime;
        int frameCount = 0;
        float fpsUpdateInterval = 1.0f; // Обновлять FPS раз в секунду
        float frameUpdateInterval = 0.01f; // Обновлять Кадр раз в 0.01 секунду
        // Жизненный цикл
        while(!glfwWindowShouldClose(window)) {

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            frameCount++;
			lastFrameTime = currentTime;

			// Передаем дельту времени в Vulkan
			vulkan.setDeltaTime(deltaTime);

            if (deltaTime >= fpsUpdateInterval) {
                std::cout << "FPS: " << frameCount / deltaTime << std::endl;
                frameCount = 0;
                lastTime = currentTime;
            }

            processInput(window);  // Вызываем обработчик ввода
            vulkan.renderFrame();// Отрисовка кадра
            glfwPollEvents();// Обработка событий
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
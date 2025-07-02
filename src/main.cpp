#include "vk.hpp"
#include <chrono>

#include <iostream>

void vkInit();

void processInput(GLFWwindow* window) {
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
		auto lastTime_fps = std::chrono::high_resolution_clock::now();
		auto lastFrameTime = lastTime_fps;
		auto lastTime_frame = lastTime_fps;
		int frameCount = 0;
		float fpsUpdateInterval = 1.0f; // Обновлять FPS раз в секунду

		// Переменные для Кадра
		const int target_fps = 1000;
		float frameUpdateInterval = 1.0f / target_fps; // Обновлять Кадр раз в 0.01 секунду
		// Жизненный цикл
		while(!glfwWindowShouldClose(window)) {

			auto currentTime = std::chrono::high_resolution_clock::now();

			float deltaTime_frame = std::chrono::duration<float>(currentTime - lastTime_frame).count();
			float deltaTime_fps = std::chrono::duration<float>(currentTime - lastTime_fps).count();
			lastFrameTime = currentTime;

			// Передаем дельту времени в Vulkan
			vulkan.setDeltaTime(deltaTime_frame);

			if (deltaTime_fps >= fpsUpdateInterval) {
				std::cout << "FPS: " << frameCount << std::endl;
				frameCount = 0;
				// glm::vec3 temp_pos = vulkan.getCameraPos();
				// std::cout << "Camera: (" << temp_pos.x << ", " << temp_pos.y << ", " << temp_pos.z << ")"<< std::endl;
				lastTime_fps = currentTime;
			}

			processInput(window);  // Вызываем обработчик ввода

			if (deltaTime_frame >= frameUpdateInterval) {
				vulkan.renderFrame();// Отрисовка кадра
				frameCount++;
				lastTime_frame = currentTime;
			}

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
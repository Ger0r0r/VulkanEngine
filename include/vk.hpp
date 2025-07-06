#ifndef VK_H
#define VK_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include "PhysicalDevice.hpp"
#include "Surface.hpp"
#include "Queue.hpp"
#include "Vertex.hpp"

class Vulkan
{
	public:
		const uint32_t size_of_field = 1000;
		void init(GLFWwindow* window); // инициализация
		void destroy(); // завершение работы
		void renderFrame(); // рендер кадра
		void setDeltaTime(float dt) { deltaTime = dt; }
		glm::vec3 getCameraPos() const ;
	private:
		VkBuffer stagingVertexBuffer;
		VkDeviceMemory stagingVertexBufferMemory;
		void* stagingVertexBufferMapped;

		VkDescriptorPool descriptorPool;
		VkDescriptorSet descriptorSet;

		void createDescriptorPool();
		void createDescriptorSet();

		GLFWwindow* window;  // Добавляем в private-секцию
		VkBuffer uniformBuffer;
		VkDeviceMemory uniformBufferMemory;
		void* uniformBufferMapped;

		VkDescriptorSetLayout descriptorSetLayout; // Для uniform buffer

		// Матрицы и камера
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 projMatrix;
		glm::vec3 cameraPos = glm::vec3(-0.5f, 0.5f, 2.0f);
		glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

		// Новые методы
		void createUniformBuffer();
		void createDescriptorSetLayout();
		void updateUniformBuffer();

		VkInstance instance; // Экземпляр Vulkan
		PhysicalDevice physicalDevice; // Физическое устройство
		VkDevice logicalDevice; // логическое устройство
		Queue queue; // очередь
		Surface surface; // Поверхность окна
		VkSwapchainKHR swapChain; // Список показа
		std::vector<VkImage> swapChainImages; // Изображения из списка показа
		std::vector<VkImageView> swapChainImageViews; // Информация об изображениях из списка показа
		std::vector<VkFramebuffer> swapChainFramebuffers; // Буферы кадра из списка показа
		VkRenderPass renderPass; // Проходы рендера
		VkPipelineLayout pipelineLayout; // Раскладка конвейера
		VkPipeline graphicsPipeline; // Графический конвейер
		VkCommandPool commandPool; // Пул команд
		std::vector<VkCommandBuffer> commandBuffers; // Буферы команд
		VkBuffer vertexBuffer; // Буфер вершин
		VkDeviceMemory vertexBufferMemory; // Память буфера вершин
		VkBuffer indexBuffer; // Буфер индексов
		VkDeviceMemory indexBufferMemory; // Память буфера индексов
		std::vector<VkSemaphore> imageAvailableSemaphores; // семафор доступности изображения
		std::vector<VkSemaphore> renderFinishedSemaphores; // семафор окончания рендера
		std::vector<VkFence> inWorkFences; // барьер кадра в работе
		uint32_t currentFrame = 0; // Текущий кадр рендера
		float animationTime = 0.0f;
		float deltaTime = 0.01f; // Примерное значение по умолчанию (60 FPS)
		std::vector<Vertex> vertices;  // Динамические вершины (вместо статичного массива)

		// Структура для хранения флагов
		struct
		{
			const bool VALIDATION = true; // Использование слоев проверки
		} states;


		void createInstance(); // Создание экземпяра Vulkan
		void selectPhysicalDevice(std::vector<const char*> &deviceExtensions); // Выбор физического устройства
		void pickQueues(); // Выбор очередей
		void createLogicalDevice(std::vector<const char*> &deviceExtensions); // Создание логического устройства
		void createWindowSurface(GLFWwindow* window); // Создание поверхности окна
		void createSwapchain(GLFWwindow* window); // Создание цепочки показа
		void createRenderpass(); // Создание проходов рендера
		VkShaderModule createShaderModule(const char * filename); // Создание шейдерного модуля
		void createGraphicPipeline(); // Создание графического конвеера
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); // Создание произвольного буфера данных
		void createCommandPool(); // Создание пула команд
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size); // Копирование между буферами данных
		void createVertexBuffer(); // Создание буфера вершин
		void createIndexBuffer(); // Создание буфера индексов
		void createSyncObjects(); // Создание объектов синхронизации
		void createFramebuffers(); // Создание буферов кадра
};

#endif // VK_H

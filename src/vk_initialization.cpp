#include "vk.hpp"
#include "Vertex.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>

#include "macroses.hpp"

// инициализация
void Vulkan::init(GLFWwindow* window) {
	createInstance(); // Создание экземпяра
	createWindowSurface(window); // Создание поверхности
	// Расширения для устройства: имена задаются внутри фигурных скобок в кавычках
	std::vector<const char*> deviceExtensions({"VK_KHR_swapchain"});
	selectPhysicalDevice(deviceExtensions); // Выбор физического устройства
	createLogicalDevice(deviceExtensions); // Создание физического устройства
	createSwapchain(window); // Создание списка показа
	createRenderpass(); // Создание проходов рендера
	createFramebuffers(); // Создание буферов кадра
	createGraphicPipeline(); // Создание графического конвейера
	createCommandPool(); // Создание пула команд
	createVertexBuffer(); // Создание буфера вершин
	createIndexBuffer(); // Создание буфера индексов
	createSyncObjects(); // Создание объектов синхронизации
}

// завершение работы
void Vulkan::destroy() {
	vkDeviceWaitIdle(logicalDevice); // Ожидание окончания асинхронных задач

	vkDestroyBuffer(logicalDevice, indexBuffer, nullptr); // Уничтожение буфера индексов
	vkFreeMemory(logicalDevice, indexBufferMemory, nullptr); // Освобождение памяти буфера индексов

	vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr); // Уничтожение буфера вершин
	vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr); // Освобождение памяти буфера вершин

	// Уничтожение объектов синхронизации
	for (int i = 0; i < surface.imageCount; i++) {
		vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(logicalDevice, inWorkFences[i], nullptr);
	}

	vkDestroyCommandPool(logicalDevice, commandPool, nullptr); // Уничтожение командного пула

	// Уничтожение буферов кадра
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
	}

	vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr); // Уничтожение графического конвейера
	vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr); // Уничтожение раскладки графического конвейера
	vkDestroyRenderPass(logicalDevice, renderPass, nullptr); // Уничтожение проходов рендера

	// Уничтожение информации о изображениях списка показа
	for (auto & imageView : swapChainImageViews) {
		vkDestroyImageView(logicalDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr); // уничтожение цепочки показа
	vkDestroySurfaceKHR(instance, surface.surface, nullptr); // уничтожение поверхности
	vkDestroyDevice(logicalDevice, nullptr); // Уничтожение логического устройства
	vkDestroyInstance(instance, nullptr); // Уничтожение экземпляра Vulkan
}

#include <cstring>
// Проверка слоев на доступность. Возвращает true, если все слои доступны
// по ссылке заполняет вектор недоступных слоев
bool checkValidationLayerSupport(std::vector <const char*> requestedLayers, std::vector <const char*> & unavailableLayers) {
	bool layerAvailable; // флаг доступности слоя для цикла

	// Первым вызовом определим кол-во доступных слоев
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Вторым вызовом запишем в вектор доступные слои
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Цикл по запрошенным слоям
	for (const char* layerName : requestedLayers) {
	   layerAvailable = false;

		// Цикл по доступным слоям
		for (const auto& layerProperties : availableLayers) {
			// Сравнение строк
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerAvailable = true;
				break;
			}
		}

		// Если слой не найден то заносим в массив недоступных
		if (!layerAvailable) {
			unavailableLayers.push_back(layerName);
		}
	}

	return unavailableLayers.size() == 0;
}

// Проверка слоев устройства на доступность. Возвращает true, если все слои доступны
// по ссылке заполняет вектор недоступных слоев
bool checkDeviceLayerSupport(VkPhysicalDevice physicalDevice, std::vector <const char*> requestedLayers, std::vector <const char*> & unavailableLayers) {
	bool layerAvailable; // флаг доступности слоя для цикла

	// Первым вызовом определим кол-во доступных слоев
	uint32_t layerCount;
	vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);

	// Вторым вызовом запишем в вектор доступные слои
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, availableLayers.data());

	// Цикл по запрошенным слоям
	for (const char* layerName : requestedLayers) {
	   layerAvailable = false;

		// Цикл по доступным слоям
		for (const auto& layerProperties : availableLayers) {
			// Сравнение строк
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerAvailable = true;
				break;
			}
		}

		// Если слой не найден то заносим в массив недоступных
		if (!layerAvailable) {
			unavailableLayers.push_back(layerName);
		}
	}

	return unavailableLayers.size() == 0;
}

void Vulkan::createInstance() {
	// Структура с данными о приложении
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Notes";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Структура с данными
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Расширения для glfw
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	// Инициализируем вектор расширений тем, что требуется для glfw
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// Подключение других расширений
	// extensions.push_back(ИМЯ_РАСШИРЕНИЯ);

	// Запишем данные об используемых расширениях в структуру
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Подключение слоев
	std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	if (states.VALIDATION) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}

	// Проверим доступность слоев
	std::vector<const char*> unavailableLayers;
	if (!checkValidationLayerSupport(validationLayers, unavailableLayers)) {
		std::cout << "Запрошены недоступные слои:\n";
		// Цикл по недоступным слоям
		for (const char* layer : unavailableLayers)
			std::cout << layer << "\n";
		// Отправим исключение об отсутствующем слое
		throw std::runtime_error("Requested layer unavailable");
	}

	// Создание экземпляра Vulkan
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
	if (result != VK_SUCCESS) {
		// Отправим исключение в случае ошибок создания экземпляра
		throw std::runtime_error("Instance create error");
	}
}

#include <algorithm>
// Выбор физического устройства на основании требований
PhysicalDevice selectPhysicalDeviceByProperties(std::vector<VkPhysicalDevice> & devices, Surface & surface, std::vector<const char*> &requestedExtensions) {
	int i;
	PhysicalDevice result; // физическое устройство (PhysicalDevice.h)
	for (const auto& device : devices) {
		// Запомним устройство
		result.device = device;
		// Получаем данные
		vkGetPhysicalDeviceProperties(device, &result.properties);
		vkGetPhysicalDeviceFeatures(device, &result.features);
		vkGetPhysicalDeviceMemoryProperties(device, &result.memory);

		// Данные по семействам очередей
		uint32_t queueFamilyPropertiesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertiesCount, nullptr);
		result.queueFamilyProperties.resize(queueFamilyPropertiesCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertiesCount, result.queueFamilyProperties.data());

		// Данные по расширениям
		uint32_t extensionsCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionsCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, extensions.data());

		int availableExtensionsCount = 0;
		// подсчитаем совпадающие расширения
		for (auto extension1 : requestedExtensions)
			for (auto extension2 : extensions)
				if (strcmp(extension1, extension2.extensionName) == 0) {
					availableExtensionsCount++;
					break;
				}

		// Получение информации о поверхности
		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface.surface, &capabilities);

		// Получение форматов поверхности
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface.surface, &formatCount, formats.data());

		// Получение данных о поддерживаемых режимах показа
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface.surface, &presentModeCount, presentModes.data());

		// Если есть форматы и режимы показа, то на данном устройстве можно создать список показа
		bool swapchainSupport = formatCount && presentModeCount;

		// Производим оценку
		if (availableExtensionsCount == requestedExtensions.size()
		&&  result.features.geometryShader
		&&  4000 < result.memory.memoryHeaps[0].size / 1000 / 1000
		&&  swapchainSupport
		) {
			// Заполним данные о поверхности
			surface.capabilities = capabilities;
			surface.formats = formats;
			surface.presentModes = presentModes;
			// Вернем устройство
			return result;
		}
	}
	// Если устройство не найдено - вернем пустую структуру
	return PhysicalDevice();
}

// Выбор физического устройства
void Vulkan::selectPhysicalDevice(std::vector<const char*> &deviceExtensions) {
	// Узнаем количество доступных устройств
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Проверка на отсутствие физических устройств
	if (deviceCount == 0) {
		throw std::runtime_error("Unable to find physical devices");
	}

	// Создадим вектор нужного размера и заполним его данными
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Выбор физического устройства на основании требований
	physicalDevice = selectPhysicalDeviceByProperties(devices, surface, deviceExtensions);

	// Если не удалось выбрать подходящее требованием устройство - выдадим исключение
	if (!physicalDevice.device) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

// Выбор очередей
void Vulkan::pickQueues() {
	queue.index = -1;

	for (int i = 0; i < physicalDevice.queueFamilyProperties.size(); i++) {
		// Проверка возможности вывода
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.device, i, surface.surface, &presentSupport);
		// Проверка поддержки очередью графических операций
		if (physicalDevice.queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
		&&  presentSupport
		) {
			queue.index = i;
			queue.properties = physicalDevice.queueFamilyProperties[i];
			break;
		}
	}
}

// Создание логического устройства
void Vulkan::createLogicalDevice(std::vector<const char*> &deviceExtensions) {
	// Выберем очереди
	pickQueues();

	// Приоритеты очередей
	float priority[1] = {1};
	// Данные о необходимых очередях
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queue.index;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = priority;

	// слои для логического устройства
	std::vector<const char*> layers;
	// Подключение других слоев
	// layers.push_back(ИМЯ_СЛОЯ);

	// Проверим доступность слоев
	std::vector<const char*> unavailableLayers;
	if (!checkDeviceLayerSupport(physicalDevice.device, layers, unavailableLayers)) {
		std::cout << "Запрошены недоступные слои:\n";
		// Цикл по недоступным слоям
		for (const char* layer : unavailableLayers)
			std::cout << layer << "\n";
		// Отправим исключение об отсутствующем слое
		throw std::runtime_error("Requested layer unavailable");
	}

	// Данные о создаваемом логическом устройстве
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.enabledExtensionCount = deviceExtensions.size();
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.enabledLayerCount = layers.size();
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.pEnabledFeatures = nullptr;//&physicalDevice.features;

	// Создание логического устройства
	if (vkCreateDevice(physicalDevice.device, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
		// Отправим исключение в случае ошибок создания лог. устройства
		throw std::runtime_error("failed to create logical device!");
	}

	// Получим дескриптор очереди логического устройства
	vkGetDeviceQueue(logicalDevice, queueCreateInfo.queueFamilyIndex, 0, &queue.descriptor);
}

// Создание поверхности окна
void Vulkan::createWindowSurface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface.surface) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create window surface");
	}
}

// Создание цепочки показа
void Vulkan::createSwapchain(GLFWwindow* window) {
	// Выбор формата
	surface.selectedFormat = surface.formats[0];
	for (auto& format : surface.formats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB
		&&  format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		) {
			surface.selectedFormat = format;
			break;
		}
	}

	// Выбор режима показа
	surface.selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (auto& presentMode : surface.presentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			surface.selectedPresentMode = presentMode;
			break;
		}
	}

	// Выбор разрешения изображений
	// Разрешение окна
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	// Выберем разрешение исходя из ограничений физического устройства
	surface.selectedExtent.width = CLAMP(surface.capabilities.minImageExtent.width, width, surface.capabilities.maxImageExtent.width);
	surface.selectedExtent.height = CLAMP(surface.capabilities.minImageExtent.height, height, surface.capabilities.maxImageExtent.height);

	// Выбор количества изображений в списке показа
	surface.imageCount = surface.capabilities.minImageCount + 1;
	// Если есть ограничение по максимуму изображений - применим его
	if (surface.capabilities.maxImageCount)
		surface.imageCount %= surface.capabilities.maxImageCount;

	// Заполнение данных о создаваемом списке показа
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface.surface;
	createInfo.minImageCount = surface.imageCount;
	createInfo.imageFormat = surface.selectedFormat.format;
	createInfo.imageColorSpace = surface.selectedFormat.colorSpace;
	createInfo.imageExtent = surface.selectedExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.preTransform = surface.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = surface.selectedPresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Создание списка показа
	if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create swap chain");
	}

	// Получение изображений списка показа
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &surface.imageCount, nullptr);
	swapChainImages.resize(surface.imageCount);
	vkGetSwapchainImagesKHR(logicalDevice, swapChain, &surface.imageCount, swapChainImages.data());

	// Зададим размер массива в соответствии с количеством изображений
	swapChainImageViews.resize(swapChainImages.size());
	// Для каждого изображения из списка показа
	for (int i = 0; i < swapChainImages.size(); i++) {
		// Заполним данные о создаваемом объекте VkImageView
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = surface.selectedFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// Создание VkImageView
		if (vkCreateImageView(logicalDevice, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Unable to create image views");
		}
	}
}

// Создание проходов рендера
void Vulkan::createRenderpass() {
	// Информация о прикреплении
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = surface.selectedFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Информация о выходном прикреплении
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Информация о подпроходе
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// Зависимости подпрохода
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Информация о создаваемом проходе рендера
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	// Создание проходов рендера
	if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create render pass");
	}

}

#include <fstream>
// Считывание бинарного файла, содержащего шейдер
void readFile(const char * filename, std::vector<char>& buffer) {
	// откроем файл как бинарный и установим курсор в конец файла
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	// если файл не открыт - генерируем исключение
	if (!file.is_open())
	{
		throw std::runtime_error("Can't open file");
	}
	// определим размер файла
	size_t fileSize = (size_t) file.tellg();
	// создадим буфер
	buffer.resize(fileSize);

	// перенесем курсор в начало файла
	file.seekg(0);
	// считаем данные в буфер
	file.read(buffer.data(), fileSize);
	// закроем файл
	file.close();
}

// Создание шейдерного модуля
VkShaderModule Vulkan::createShaderModule(const char * filename) {
	// буфер для чтения из файла
	std::vector<char> buffer;
	// считаем шейдер из файла
	readFile(filename, buffer);

	// Информация о создаваемом шейдерном модуле
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());

	// Создание шейдерного модуля
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create shader module");
	}

	return shaderModule;
}

// Создание графического конвеера
void Vulkan::createGraphicPipeline() {
	// Входные данные вершин
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// Привязка
	VkVertexInputBindingDescription bindingDescription{};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Описание атрибута
	VkVertexInputAttributeDescription attributeDescriptions[2];

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, color);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = 2;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	// Входной сборщик
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Область просмотра
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = surface.selectedExtent.width;
	viewport.height = surface.selectedExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Прямоугольник отсечения
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = surface.selectedExtent;

	// Состояние области просмотра
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Растеризатор
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	// Мультисэмплинг
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Смешивание цветов для буфера
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	// Глобальные настройки смешивания цветов
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// раскладка конвейера
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Unable to create pipeline layout");
	}

	// Создание шейдеров
	VkShaderModule vertShaderModule = createShaderModule("build/shaders/vert.spv");
	VkShaderModule fragShaderModule = createShaderModule("build/shaders/frag.spv");

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	// Шейдерные стадии
	VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};


	// Информация о создаваемом конвейере
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	// Создание графического конвейера
	if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create graphics pipeline");
	}

	// Удаление шейдерных модулей
	vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
}

// Создание произвольного буфера данных
void Vulkan::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	// Информация о создаваемом буфере
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Создание буфера
	if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create buffer");
	}

	// Требования к памяти
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

	// Поиск индекса типа подходящей памяти
	uint32_t index_memory;
	for (index_memory = 0; index_memory < physicalDevice.memory.memoryTypeCount; index_memory++) {
		if ((memRequirements.memoryTypeBits & (1 << index_memory))
		&&  (physicalDevice.memory.memoryTypes[index_memory].propertyFlags & properties) == properties
		) {
			break;
		}
	}

	// Если индекс равен размеру массива - поиск не удался и нужно выдать исключение
	if (index_memory == physicalDevice.memory.memoryTypeCount)
		throw std::runtime_error("Unable to find suitable memory type");

	// Информация о выделяемой памяти
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = index_memory;

	// Выделение памяти
	if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Unable to allocate buffer memory");
	}

	// Привязка выделенной памяти к буферу
	vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
}

// Создание пула команд
void Vulkan::createCommandPool() {
	// Информация о создаваемом командном пуле
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queue.index;

	// Создание командного пула
	if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("Unable to create graphics command pool");
	}

	// Выделим память под буферы команд
	commandBuffers.resize(swapChainImages.size());

	// Информация о выделяемых буферах
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	// Выделение буферов команд из пула команд
	if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Unable to allocate command buffers");
	}
}

// Копирование между буферами данных
void Vulkan::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	// Информация о выделяемом буфере команд
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	// Дескриптор и выделение буфера команд
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

	// Начало записи команд
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Операция копирования
	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	// Конец записи команд
	vkEndCommandBuffer(commandBuffer);

	// Информация о запускаемых буферах команд
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	// Запуск командного буфера копирования и ожидание завершения
	vkQueueSubmit(queue.descriptor, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue.descriptor);

	// Освобождение командного буфера копирования
	vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

// Создание вершинного буфера
void Vulkan::createVertexBuffer() {
	// Исходные вершины (цвета остаются теми же)
	vertices = {
		{ { 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f} },  // Красный
		{ { 0.5f,  0.5f}, {0.0f, 1.0f, 0.0f} },  // Зелёный
		{ {-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} },   // Синий
		{ { 0.0f,  1.0f}, {1.0f, 1.0f, 1.0f} },   // Белый

		{ { 0.9f, -0.9f}, {1.0f, 0.0f, 0.0f} },
		{ { 0.95f,-0.85f},{1.0f, 0.0f, 0.0f} },
		{ { 0.9f, -0.8f}, {1.0f, 0.0f, 0.0f} },
		{ { 0.85f,-0.75f},{1.0f, 0.0f, 0.0f} },
		{ { 0.8f, -0.8f}, {1.0f, 0.0f, 0.0f} },
		{ { 0.75f,-0.85f},{1.0f, 0.0f, 0.0f} },
		{ { 0.8f, -0.9f}, {1.0f, 0.0f, 0.0f} },
		{ { 0.85f,-0.95f},{1.0f, 0.0f, 0.0f} }
	};

	// Создаём буфер (как раньше)
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
	createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	// Копируем данные в буфер (через staging buffer, как в оригинале)
	updateVertexBuffer();  // Новая функция для обновления данных
}

// Создание буфера индексов
void Vulkan::createIndexBuffer() {
	// Индексы, записываемые в буфер
	uint16_t indices[] = {0,1,2, 1,3,2, 4,5,6, 6,7,8, 8,9,10, 10,11,4};
	// Размер буфера в байтах
	VkDeviceSize bufferSize = sizeof(uint16_t) * 3 * 6;

	// Промежуточный буфер для переноса на устройство
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Отображение памяти буфера
	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	// Копирование вершин в промежуточный буфер
	memcpy(data, indices, (size_t) bufferSize);
	// Прекращение отображения памяти буфера
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	// Создание буфера вершин
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
	// Копирование из промежуточного в буфер вершин
	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	// Освобождение памяти и уничтожение буферов
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

// Создание объектов синхронизации
void Vulkan::createSyncObjects() {
	imageAvailableSemaphores.resize(surface.imageCount);
	renderFinishedSemaphores.resize(surface.imageCount);
	inWorkFences.resize(surface.imageCount);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < surface.imageCount; i++) {
		if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS
		||  vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS
		||  vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inWorkFences[i]) != VK_SUCCESS
		) {
			throw std::runtime_error("Unable to create synchronization objects for frame");
		}
	}
}

// Создание буферов кадра
void Vulkan::createFramebuffers() {
	// Зададим размер массива в соответствии с количеством изображений
	swapChainFramebuffers.resize(swapChainImageViews.size());
	// Для каждого изображения из списка показа
	for (int i = 0; i < swapChainImageViews.size(); i++) {
		// Изображения используемые в буфере кадра
		VkImageView attachments[] = { swapChainImageViews[i] };
		// Заполним данные о создаваемом буфере кадра
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = surface.selectedExtent.width;
		framebufferInfo.height = surface.selectedExtent.height;
		framebufferInfo.layers = 1;

		// Создание буфера кадра
		if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Unable to create framebuffer");
		}
	}
}


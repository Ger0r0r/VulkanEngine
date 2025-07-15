#include "vk.hpp"
#include "Vertex.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>
#include <array>  // Для std::array

#include "macroses.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>



// инициализация
void Vulkan::init(GLFWwindow* window) {
	this->window = window;
	createInstance(); // Создание экземпяра
	createWindowSurface(window); // Создание поверхности
	// Расширения для устройства: имена задаются внутри фигурных скобок в кавычках
	std::vector<const char*> deviceExtensions({"VK_KHR_swapchain"});
	selectPhysicalDevice(deviceExtensions); // Выбор физического устройства
	createLogicalDevice(deviceExtensions); // Создание физического устройства
	createSwapchain(window); // Создание списка показа
    createDepthResources(); // Добавить эту строку
	createRenderpass(); // Создание проходов рендера
	createFramebuffers(); // Создание буферов кадра
	createDescriptorSetLayout(); // <- Добавляем эту строку
	createGraphicPipeline(); // Создание графического конвейера
	createCommandPool(); // Создание пула команд
	createTextureImage();
	createVertexBuffer(); // Создание буфера вершин
	createIndexBuffer(); // Создание буфера индексов
	createUniformBuffer(); // <- Добавляем эту строку
	createDescriptorPool();    // Добавьте эту строку
	createDescriptorSet();     // Добавьте эту строку
	createSyncObjects(); // Создание объектов синхронизации
	loadModel("models/Model.fbx"); // Укажите путь к модели
    createModelBuffers();
}

void Vulkan::createTextureImage() {
    // 1. Загрузка изображения из файла
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("models/ork_body_D.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4; // 4 байта на пиксель (RGBA)

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // 2. Создание промежуточного буфера
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(imageSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                stagingBuffer,
                stagingBufferMemory);

    // 3. Копирование данных в буфер
    void* data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    // 4. Освобождение памяти изображения
    stbi_image_free(pixels);

    // 5. Создание VkImage для текстуры
    createImage(texWidth, texHeight,
               VK_FORMAT_R8G8B8A8_SRGB,
               VK_IMAGE_TILING_OPTIMAL,
               VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
               textureImage,
               textureImageMemory);

    // 6. Переход изображения в оптимальный layout для копирования
    transitionImageLayout(textureImage,
                         VK_FORMAT_R8G8B8A8_SRGB,
                         VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // 7. Копирование из буфера в изображение
    copyBufferToImage(stagingBuffer,
                     textureImage,
                     static_cast<uint32_t>(texWidth),
                     static_cast<uint32_t>(texHeight));

    // 8. Переход в layout для шейдерного чтения
    transitionImageLayout(textureImage,
                         VK_FORMAT_R8G8B8A8_SRGB,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // 9. Очистка staging буферов
    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);

    // 10. Создание image view
    createImageView(textureImage,
                   VK_FORMAT_R8G8B8A8_SRGB,
                   VK_IMAGE_ASPECT_COLOR_BIT,
                   &textureImageView);

    // 11. Создание сэмплера текстуры
    createTextureSampler();
}

uint32_t Vulkan::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice.device, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Vulkan::createImageView(VkImage image, VkFormat format,
                           VkImageAspectFlags aspectFlags,
                           VkImageView* imageView) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
}

VkCommandBuffer Vulkan::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Vulkan::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue.descriptor, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue.descriptor);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
}

void Vulkan::createImage(uint32_t width, uint32_t height, VkFormat format,
                        VkImageTiling tiling, VkImageUsageFlags usage,
                        VkMemoryPropertyFlags properties,
                        VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(logicalDevice, image, imageMemory, 0);
}

void Vulkan::transitionImageLayout(VkImage image, VkFormat format,
                                  VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void Vulkan::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}

void Vulkan::createTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    if (vkCreateSampler(logicalDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void Vulkan::createModelBuffers() {
    // 1. Vertex Buffer
    VkDeviceSize vertexBufferSize = sizeof(Vertex) * modelVertices.size();

    // Создаем промежуточный буфер (staging)
    VkBuffer stagingVertexBuffer;
    VkDeviceMemory stagingVertexBufferMemory;
    createBuffer(vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingVertexBuffer, stagingVertexBufferMemory);

    // Копируем данные в staging буфер
    void* vertexData;
    vkMapMemory(logicalDevice, stagingVertexBufferMemory, 0, vertexBufferSize, 0, &vertexData);
    memcpy(vertexData, modelVertices.data(), vertexBufferSize);
    vkUnmapMemory(logicalDevice, stagingVertexBufferMemory);

    // Создаем конечный vertex buffer на GPU
    createBuffer(vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        modelVertexBuffer, modelVertexBufferMemory);

    // Копируем из staging в GPU буфер
    copyBuffer(stagingVertexBuffer, modelVertexBuffer, vertexBufferSize);

    // 2. Index Buffer (аналогично)
    VkDeviceSize indexBufferSize = sizeof(uint32_t) * modelIndices.size();

    VkBuffer stagingIndexBuffer;
    VkDeviceMemory stagingIndexBufferMemory;
    createBuffer(indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingIndexBuffer, stagingIndexBufferMemory);

    void* indexData;
    vkMapMemory(logicalDevice, stagingIndexBufferMemory, 0, indexBufferSize, 0, &indexData);
    memcpy(indexData, modelIndices.data(), indexBufferSize);
    vkUnmapMemory(logicalDevice, stagingIndexBufferMemory);

    createBuffer(indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        modelIndexBuffer, modelIndexBufferMemory);

    copyBuffer(stagingIndexBuffer, modelIndexBuffer, indexBufferSize);

    // 3. Очищаем staging буферы
    vkDestroyBuffer(logicalDevice, stagingVertexBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingVertexBufferMemory, nullptr);
    vkDestroyBuffer(logicalDevice, stagingIndexBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingIndexBufferMemory, nullptr);
}


void Vulkan::createUniformBuffer() {
	VkDeviceSize bufferSize = sizeof(glm::mat4) * 3 + sizeof(float) + sizeof(int); // model, view, proj + time

	createBuffer(bufferSize,
			   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			   uniformBuffer,
			   uniformBufferMemory);

	vkMapMemory(logicalDevice, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);
}

void Vulkan::createDescriptorSetLayout() {
    std::vector<VkDescriptorSetLayoutBinding> bindings;

    // Uniform buffer (binding 0)
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(uboLayoutBinding);

    // Texture sampler (binding 1)
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    bindings.push_back(samplerLayoutBinding);

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

// завершение работы
void Vulkan::destroy() {
	vkDeviceWaitIdle(logicalDevice); // Ожидание окончания асинхронных задач

	vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);

	vkDestroyBuffer(logicalDevice, modelVertexBuffer, nullptr);
	vkFreeMemory(logicalDevice, modelVertexBufferMemory, nullptr);
	vkDestroyBuffer(logicalDevice, modelIndexBuffer, nullptr);
	vkFreeMemory(logicalDevice, modelIndexBufferMemory, nullptr);

	vkDestroySampler(logicalDevice, textureSampler, nullptr);
	vkDestroyImageView(logicalDevice, textureImageView, nullptr);
	vkDestroyImage(logicalDevice, textureImage, nullptr);
	vkFreeMemory(logicalDevice, textureImageMemory, nullptr);

    vkDestroyImageView(logicalDevice, depthImageView, nullptr);
    vkDestroyImage(logicalDevice, depthImage, nullptr);
    vkFreeMemory(logicalDevice, depthImageMemory, nullptr);

	// Уничтожаем uniform buffer
	vkDestroyBuffer(logicalDevice, uniformBuffer, nullptr);
	vkFreeMemory(logicalDevice, uniformBufferMemory, nullptr);

	// Уничтожаем layout дескрипторов
	vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);


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

    // Проверим доступность слоев
    std::vector<const char*> unavailableLayers;
    if (!checkDeviceLayerSupport(physicalDevice.device, layers, unavailableLayers)) {
        std::cout << "Запрошены недоступные слои:\n";
        for (const char* layer : unavailableLayers)
            std::cout << layer << "\n";
        throw std::runtime_error("Requested layer unavailable");
    }

    // Включим фичу анизотропной фильтрации
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;  // ← Это важно!

    // Данные о создаваемом логическом устройстве
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.pEnabledFeatures = &deviceFeatures;  // ← Передаем фичи сюда

    // Создание логического устройства
    if (vkCreateDevice(physicalDevice.device, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
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
    // Attachment для цвета
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = surface.selectedFormat.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Attachment для глубины
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // References
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Dependencies
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Массив attachments
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    // Render pass info
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

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

void Vulkan::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();

    createImage(
        surface.selectedExtent.width,
        surface.selectedExtent.height,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage,
        depthImageMemory
    );

    createImageView(
        depthImage,
        depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &depthImageView
    );

    transitionImageLayout(
        depthImage,
        depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    );
}

VkFormat Vulkan::findDepthFormat() {
    return VK_FORMAT_D32_SFLOAT; // Или другой подходящий формат
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

    // Описание атрибута (только позиция)
	VkVertexInputAttributeDescription attributeDescriptions[3] = {};
	attributeDescriptions[0] = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)};
	attributeDescriptions[1] = {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)};
	attributeDescriptions[2] = {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord)};
	vertexInputInfo.vertexAttributeDescriptionCount = 3;

    vertexInputInfo.vertexBindingDescriptionCount = 1;
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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
	pipelineLayoutInfo.setLayoutCount = 1;  // ← Количество макетов дескрипторов
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;  // ← Ваш макет дескриптора
	pipelineLayoutInfo.pushConstantRangeCount = 0;  // (если не используете push-константы)

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

    // Добавляем состояние глубины
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;

	// Информация о создаваемом конвейере
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pDepthStencilState = &depthStencil;
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
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queue.index;  // ← Убедитесь, что queue.index установлен правильно

    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Unable to create graphics command pool");
    }

    // Выделение буферов команд
    commandBuffers.resize(swapChainImages.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;  // ← Используем созданный commandPool
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

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
	vertices.clear();
	for (int i = 0; i < size_of_field; i++) {
		for (int j = 0; j < size_of_field; j++) {
			vertices.push_back({
				{((float)i)/size_of_field, ((float)j)/size_of_field, 0.0f}  // Z будет вычисляться в шейдере
			});
		}
	}

	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
	createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

	// Теперь updateVertexBuffer() не нужен, так как анимация полностью в шейдере
	void* data;
	vkMapMemory(logicalDevice, vertexBufferMemory, 0, bufferSize, 0, &data);
	// Копирование вершин в промежуточный буфер
	memcpy(data, vertices.data(), (size_t) bufferSize);
	// Прекращение отображения памяти буфера
	vkUnmapMemory(logicalDevice, vertexBufferMemory);
}

// Создание буфера индексов
void Vulkan::createIndexBuffer() {

	std::vector<uint32_t> indices((size_of_field-1)*(size_of_field-1)*6);

	for (int i = 0; i < (size_of_field-1); i++) {
		for (int j = 0; j < (size_of_field-1); j++) {
			indices[0 + 6 * (i * (size_of_field-1) + j)] = i * size_of_field + j;
			indices[1 + 6 * (i * (size_of_field-1) + j)] = (i + 1) * size_of_field + j;
			indices[2 + 6 * (i * (size_of_field-1) + j)] = (i + 1) * size_of_field + j + 1;
			indices[3 + 6 * (i * (size_of_field-1) + j)] = i * size_of_field + j;
			indices[4 + 6 * (i * (size_of_field-1) + j)] = (i + 1) * size_of_field + j + 1;
			indices[5 + 6 * (i * (size_of_field-1) + j)] = i * size_of_field + j + 1;
		}
	}

	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

	// Промежуточный буфер для переноса на устройство
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	// Отображение памяти буфера
	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	// Копирование вершин в промежуточный буфер
	memcpy(data, indices.data(), (size_t) bufferSize);
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
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i],
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = surface.selectedExtent.width;
        framebufferInfo.height = surface.selectedExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Unable to create framebuffer");
        }
    }
}

void Vulkan::createDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Vulkan::createDescriptorSet() {
    VkDescriptorSetLayout layouts[] = {descriptorSetLayout};
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(logicalDevice, &allocInfo, &descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    // Uniform buffer
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(glm::mat4) * 3 + sizeof(float);

    // Texture sampler
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureImageView;
    imageInfo.sampler = textureSampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = descriptorSet;
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(descriptorWrites.size()),
                         descriptorWrites.data(), 0, nullptr);
}
#include "vk.hpp"
#include "Vertex.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>

#include "macroses.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>  // Для rotate, lookAt, perspective
#include <glm/gtc/type_ptr.hpp>         // Для работы с матрицами

glm::vec3 hsvToRgb(glm::vec3 in)
{
    double      hh, p, q, t, ff;
    long        i;
    glm::vec3         out;
    if(in.y <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.z;
        out.g = in.z;
        out.b = in.z;
        return out;
    }
    hh = in.x;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.z * (1.0 - in.y);
    q = in.z * (1.0 - (in.y * ff));
    t = in.z * (1.0 - (in.y * (1.0 - ff)));
    switch(i) {
    case 0:
        out.r = in.z;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.z;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.z;
        out.b = t;
        break;
    case 3:
        out.r = p;
        out.g = q;
        out.b = in.z;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.z;
        break;
    case 5:
    default:
        out.r = in.z;
        out.g = p;
        out.b = q;
        break;
    }
    return out;
}

void Vulkan::updateVertexBuffer() {

	for (int i = 0; i < size_of_field; i++) {
		for (int j = 0; j < size_of_field; j++) {
			float current_color = animationTime * 10 + (i - j) * 200 / size_of_field;
			current_color -= 360.0f * std::floor(current_color / 360.0f);

			glm::vec3 rgb = hsvToRgb({current_color, 1.0f, 1.0f});

			vertices[i*size_of_field+j].position.z = sin(animationTime + (i - j) * 10.0f / size_of_field)*0.1f;
			vertices[i*size_of_field+j].color.x = rgb.x;
			vertices[i*size_of_field+j].color.y = rgb.y;
			vertices[i*size_of_field+j].color.z = rgb.z;
		}
	}

	// 2. Копируем обновлённые вершины в GPU-буфер
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

	// Создаём временный staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingBufferMemory);

	// Копируем вершины в staging buffer
	void* data;
	vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), bufferSize);
	vkUnmapMemory(logicalDevice, stagingBufferMemory);

	// Копируем из staging в основной буфер
	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	// Удаляем staging buffer
	vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}
// Рендер кадра
void Vulkan::renderFrame() {
    // 1. Обновляем камеру (WASD + пробел/Ctrl)
    const float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraUp;

    // 2. Обновляем матрицы
    modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    projMatrix = glm::perspective(glm::radians(45.0f),
                                 surface.selectedExtent.width / (float)surface.selectedExtent.height,
                                 0.1f, 100.0f);
    projMatrix[1][1] *= -1; // Инвертируем Y для Vulkan

    // 3. Копируем матрицы в uniform buffer
    memcpy(uniformBufferMapped, &modelMatrix, sizeof(glm::mat4));
    memcpy((char*)uniformBufferMapped + sizeof(glm::mat4), &viewMatrix, sizeof(glm::mat4));
    memcpy((char*)uniformBufferMapped + 2*sizeof(glm::mat4), &projMatrix, sizeof(glm::mat4));

	// 1. Обновляем таймер анимации
	animationTime += deltaTime;  // Скорость анимации

	// 2. Обновляем вершины
	updateVertexBuffer();
	vkWaitForFences(logicalDevice, 1, &inWorkFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(logicalDevice, 1, &inWorkFences[currentFrame]);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Unable to acquire swap chain image");
	}

	vkResetCommandBuffer(commandBuffers[currentFrame], 0);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Unable to begin recording command buffer");
	}

	VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = surface.selectedExtent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkBuffer vertexBuffers[] = {vertexBuffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffers[currentFrame], 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(commandBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    // Перед vkCmdDrawIndexed добавьте:
    vkCmdBindDescriptorSets(commandBuffers[currentFrame],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipelineLayout,
                          0, 1, &descriptorSet,
                          0, nullptr);

    vkCmdDrawIndexed(commandBuffers[currentFrame], (size_of_field-1)*(size_of_field-1)*6, 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffers[currentFrame]);

	if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("Unable to record command buffer");
	}

	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(queue.descriptor, 1, &submitInfo, inWorkFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("Unable to submit draw command buffer");
	}

	currentFrame = (currentFrame + 1) % surface.imageCount;

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChain;
	presentInfo.pImageIndices = &imageIndex;

	if (vkQueuePresentKHR(queue.descriptor, &presentInfo) != VK_SUCCESS) {
		throw std::runtime_error("Unable to present swap chain image");
	}
}

glm::vec3 Vulkan::getCameraPos() const {
    return cameraPos;
}
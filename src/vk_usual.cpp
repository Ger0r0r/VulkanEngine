#include "vk.hpp"
#include "Vertex.hpp"

#include <iostream>
#include <vector>
#include <stdexcept>

#include "macroses.hpp"

void Vulkan::updateVertexBuffer() {
	// 1. Обновляем позиции вершин по синусоиде
	float offset = sin(animationTime) * 0.5f;  // Движение влево-вправо (-0.5..0.5)
	vertices[0].position.x = offset;          // Вершина 1
	vertices[1].position.x = 0.5f + offset;   // Вершина 2
	vertices[2].position.x = -0.5f + offset;  // Вершина 3

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
void Vulkan::renderFrame(bool key_w, bool key_a, bool key_s, bool key_d) {
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

	vkCmdBindIndexBuffer(commandBuffers[currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(commandBuffers[currentFrame], 6, 1, 0, 0, 0);

	if (key_w) {
		vkCmdDrawIndexed(commandBuffers[currentFrame], 3, 1, 15, 0, 0);
	}
	if (key_a) {
		vkCmdDrawIndexed(commandBuffers[currentFrame], 3, 1, 12, 0, 0);
	}
	if (key_s) {
		vkCmdDrawIndexed(commandBuffers[currentFrame], 3, 1, 9, 0, 0);
	}
	if (key_d) {
		vkCmdDrawIndexed(commandBuffers[currentFrame], 3, 1, 6, 0, 0);
	}

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

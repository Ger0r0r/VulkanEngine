#ifndef PHYSICALDEVICE_H
#define PHYSICALDEVICE_H

#include <vulkan/vulkan.h>

#include <vector>

typedef struct _PhysicalDevice
{
    VkPhysicalDevice device; // устройство
    VkPhysicalDeviceProperties properties; // параметры
    VkPhysicalDeviceFeatures features; // функции
    VkPhysicalDeviceMemoryProperties memory; // память
    std::vector<VkQueueFamilyProperties> queueFamilyProperties; // семейства очередей
} PhysicalDevice;

#endif // PHYSICALDEVICE_H

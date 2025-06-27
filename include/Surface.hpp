#ifndef SURFACE_H
#define SURFACE_H

#include <vulkan/vulkan.h>

#include <vector>

typedef struct _Surface
{
    VkSurfaceKHR surface; // Поверхность окна
    VkSurfaceCapabilitiesKHR capabilities; // общая информация
    std::vector<VkSurfaceFormatKHR> formats; // формат поверхности
    std::vector<VkPresentModeKHR> presentModes; // режим показа
    // Данные о списке показа
    VkSurfaceFormatKHR selectedFormat; // выбранный формат поверхности
    VkPresentModeKHR selectedPresentMode; // выбранный режим показа
    VkExtent2D selectedExtent; // выбранное разрешение
    uint32_t imageCount; // количество изображений
} Surface;

#endif // SURFACE_H

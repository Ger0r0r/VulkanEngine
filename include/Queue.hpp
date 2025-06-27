#ifndef QUEUE_H
#define QUEUE_H

#include <vulkan/vulkan.h>

typedef struct _Queue
{
    uint32_t index;
    VkQueue descriptor;
    VkQueueFamilyProperties properties;
} Queue;

#endif // QUEUE_H

#ifndef VERTEX_H
#define VERTEX_H

#include <GLM/glm.hpp>

typedef struct _Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
} Vertex;

#endif // VERTEX_H

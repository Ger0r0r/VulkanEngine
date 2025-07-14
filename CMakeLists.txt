cmake_minimum_required(VERSION 3.10)
project(VulkanTriangle)

add_link_options(-static -static-libgcc -static-libstdc++)

add_subdirectory(external/glfw)

include_directories(include)

find_package(Vulkan REQUIRED)

file(GLOB CPPS "src/*.cpp")

add_executable(VulkanTriangle ${CPPS})
target_link_libraries(VulkanTriangle glfw Vulkan::Vulkan)
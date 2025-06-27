cmake_minimum_required(VERSION 3.10)
project(VulkanTriangle)

set(VULKAN_SDK "C:/VulkanSDK/1.4.313.2")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/Include")
set(Vulkan_LIBRARY "${VULKAN_SDK}/Lib/vulkan-1.lib")

set(CMAKE_PREFIX_PATH "C:/msys64/mingw64")

add_link_options(-static -static-libgcc -static-libstdc++)

add_subdirectory(external/glfw)

include_directories(include)

find_package(Vulkan REQUIRED)

file(GLOB CPPS "src/*.cpp")

add_executable(VulkanTriangle ${CPPS})
target_link_libraries(VulkanTriangle glfw Vulkan::Vulkan)
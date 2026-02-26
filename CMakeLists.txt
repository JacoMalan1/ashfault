cmake_minimum_required(VERSION 3.12)
project(AshFault)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(CMAKE_CXX_STANDARD 20)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(ashfault src/main.cpp ${SRC_FILES})

target_include_directories(ashfault PRIVATE include)
target_include_directories(ashfault PRIVATE vendor/glfw/include)
target_include_directories(ashfault PRIVATE vendor/VulkanMemoryAllocator/include)
target_include_directories(ashfault PRIVATE vendor/spdlog/include)

target_compile_options(ashfault PUBLIC -Wall -Wpedantic -Wno-nullability-extension -Wno-nullability-completeness)

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/VulkanMemoryAllocator)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)

target_link_libraries(ashfault glfw)
target_link_libraries(ashfault vulkan)
target_link_libraries(ashfault GPUOpen::VulkanMemoryAllocator)
target_link_libraries(ashfault spdlog::spdlog)

cmake_minimum_required(VERSION 3.12)
project(AshFault)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

set(CMAKE_CXX_STANDARD 17)
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB SHADERS ${SHADER_SOURCE_DIR}/*.vert ${SHADER_SOURCE_DIR}/*.frag)

add_custom_command(
  COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}")

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND "glslc" "-o" "${SHADER_BINARY_DIR}/${FILENAME}.spv" "${source}"
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}")
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

add_executable(ashfault src/main.cpp ${SRC_FILES})

target_include_directories(ashfault PRIVATE include)
target_include_directories(ashfault PRIVATE vendor/glfw/include)
target_include_directories(ashfault PRIVATE vendor/VulkanMemoryAllocator/include)
target_include_directories(ashfault PRIVATE vendor/spdlog/include)
target_include_directories(ashfault PUBLIC vendor/CLSTL/include)

target_compile_options(ashfault PUBLIC -Wall -Wpedantic -Wno-nullability-extension -Wno-nullability-completeness)

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/VulkanMemoryAllocator)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/CLSTL)

target_link_libraries(ashfault glfw)
target_link_libraries(ashfault vulkan)
target_link_libraries(ashfault GPUOpen::VulkanMemoryAllocator)
target_link_libraries(ashfault spdlog::spdlog)
target_link_libraries(ashfault clstl)

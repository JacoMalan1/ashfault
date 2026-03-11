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

file(GLOB_RECURSE LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB IMGUI_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/imgui/*.cpp)
file(GLOB IMGUIZMO_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/*.cpp)
list(APPEND IMGUI_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_vulkan.cpp)
list(APPEND IMGUI_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/editor/main.cpp)
file(GLOB SHADERS ${SHADER_SOURCE_DIR}/*.vert ${SHADER_SOURCE_DIR}/*.frag)

find_package(Doxygen)
find_package(Vulkan REQUIRED)

if(DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

  configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
  message("Doxygen build started")

  add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} -q ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM)
else(DOXYGEN_FOUND)
  message("Doxygen is required to generate documentation")
endif(DOXYGEN_FOUND)

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

add_library(tinyobjloader vendor/tinyobjloader/tiny_obj_loader.cc vendor/tinyobjloader/tiny_obj_loader.h)
add_library(ashfault ${LIB_SRC_FILES} ${IMGUI_SRC_FILES} ${LIB_HEADERS} ${IMGUIZMO_SRC_FILES})
add_executable(program ${PROJECT_SOURCE_DIR}/src/main.cpp)
add_executable(editor ${PROJECT_SOURCE_DIR}/src/editor/main.cpp)

include_directories(include)
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(vendor/glfw/include)
include_directories(vendor/VulkanMemoryAllocator/include)
include_directories(vendor/spdlog/include)
include_directories(vendor/glm)
include_directories(vendor/tinyobjloader)
include_directories(vendor/imgui)
include_directories(vendor/imgui/backends)
include_directories(vendor/ImGuizmo)

if(NOT(MSVC))
  target_compile_options(ashfault PUBLIC -Wall -Wpedantic -Wno-nullability-extension -Wno-nullability-completeness)
endif(NOT(MSVC))

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/VulkanMemoryAllocator)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glm)

target_link_libraries(ashfault glfw)
target_link_libraries(ashfault Vulkan::Vulkan)
target_link_libraries(ashfault GPUOpen::VulkanMemoryAllocator)
target_link_libraries(ashfault spdlog::spdlog)
target_link_libraries(ashfault glm::glm)
target_link_libraries(ashfault tinyobjloader)

target_link_libraries(editor ashfault)

target_link_libraries(program tinyobjloader)
target_link_libraries(program ashfault)

configure_file(assets/models/monkey.obj monkey.obj COPYONLY)

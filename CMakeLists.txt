cmake_minimum_required(VERSION 3.12)
project(AshFault)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/flatbuffers)
file(GLOB SCHEMAS ${PROJECT_SOURCE_DIR}/schemas/*.fbs)

set(FB_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${FB_GENERATED_DIR})

set(GENERATED_HEADERS)

foreach(SCHEMA ${SCHEMAS})
  get_filename_component(SCHEMA_NAME ${SCHEMA} NAME_WE)
  set(OUT_HEADER ${FB_GENERATED_DIR}/${SCHEMA_NAME}_generated.h)

  add_custom_command(
    OUTPUT ${OUT_HEADER}
    COMMAND flatc --cpp -o ${FB_GENERATED_DIR} ${SCHEMA}
    DEPENDS flatc ${SCHEMA}
    COMMENT "Compiling FlatBuffers schema: ${SCHEMA_NAME}"
    VERBATIM
  )

  list(APPEND GENERATED_HEADERS ${OUT_HEADER})
endforeach()

add_custom_target(generate_schemas
  DEPENDS ${GENERATED_HEADERS}
)

option(ASHFAULT_SANITIZE "Enable sanitizers" OFF)

if (ASHFAULT_SANITIZE)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=address,undefined)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_compile_definitions(ASHFAULT_VK_VALIDATION=1)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(ASSIMP_BUILD_ZLIB ON)

file(GLOB_RECURSE LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB LUA_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/lua/*.c)
file(GLOB IMGUI_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/imgui/*.cpp ${PROJECT_SOURCE_DIR}/vendor/imgui/misc/cpp/imgui_stdlib.cpp)
file(GLOB IMGUIZMO_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/ImGuizmo/*.cpp)
list(APPEND IMGUI_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_vulkan.cpp)
list(APPEND IMGUI_SRC_FILES ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${PROJECT_SOURCE_DIR}/src/editor/main.cpp)
file(GLOB SHADERS ${SHADER_SOURCE_DIR}/*.vert ${SHADER_SOURCE_DIR}/*.frag)
list(APPEND LIB_SRC_FILES ${GENERATED_HEADERS})

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

add_library(lua STATIC ${LUA_SRC_FILES})
add_library(ashfault SHARED ${LIB_SRC_FILES} ${IMGUI_SRC_FILES} ${LIB_HEADERS} ${IMGUIZMO_SRC_FILES})
add_dependencies(ashfault generate_schemas)
target_compile_definitions(ashfault PUBLIC -DSPDLOG_ACTIVE_LEVEL=0)
target_compile_definitions(ashfault PUBLIC -DASHFAULT_BUILD_DLL)

add_executable(program ${PROJECT_SOURCE_DIR}/src/main.cpp)
add_executable(editor ${PROJECT_SOURCE_DIR}/src/editor/main.cpp ${PROJECT_SOURCE_DIR}/src/renderer/vma_usage.cpp)

include_directories(SYSTEM include)
include_directories(SYSTEM ${Vulkan_INCLUDE_DIRS})
include_directories(SYSTEM vendor/glfw/include)
include_directories(SYSTEM vendor/VulkanMemoryAllocator/include)
include_directories(SYSTEM vendor/spdlog/include)
include_directories(SYSTEM vendor/glm)
include_directories(SYSTEM vendor/imgui)
include_directories(SYSTEM vendor/imgui/backends)
include_directories(SYSTEM vendor/ImGuizmo)
include_directories(SYSTEM vendor/sol2/include)
include_directories(SYSTEM vendor/lua)
include_directories(SYSTEM vendor/stb)
include_directories(SYSTEM vendor/flatbuffers/include)

if(NOT(MSVC))
  target_compile_options(ashfault PUBLIC -Wall -Wpedantic -Wno-nullability-extension -Wno-nullability-completeness)
endif(NOT(MSVC))

if(WIN32)
  add_definitions(-DASHFAULT_PLATFORM_WINDOWS)
endif()

if(UNIX)
  add_definitions(-DASHFAULT_PLATFORM_LINUX)
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/VulkanMemoryAllocator)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/assimp)
add_subdirectory(${PROJECT_SOURCE_DIR}/vendor/sol2)

target_link_libraries(ashfault PUBLIC glfw)
target_link_libraries(ashfault PUBLIC Vulkan::Vulkan)
target_link_libraries(ashfault PUBLIC GPUOpen::VulkanMemoryAllocator)
target_link_libraries(ashfault PUBLIC spdlog::spdlog)
target_link_libraries(ashfault PUBLIC glm::glm)
target_link_libraries(ashfault PUBLIC assimp)
target_link_libraries(ashfault PUBLIC lua)
target_link_libraries(ashfault PUBLIC sol2)

target_include_directories(ashfault PUBLIC ${FB_GENERATED_DIR})

target_link_libraries(editor ashfault)
target_link_libraries(editor GPUOpen::VulkanMemoryAllocator)

target_link_libraries(program ashfault)
target_link_libraries(program GPUOpen::VulkanMemoryAllocator)

configure_file(assets/models/monkey.obj monkey.obj COPYONLY)
configure_file(assets/models/torus.obj torus.obj COPYONLY)
configure_file(assets/scripts/script.lua script.lua COPYONLY)
configure_file(assets/fonts/Roboto/static/Roboto-Regular.ttf Roboto-Regular.ttf COPYONLY)
configure_file(assets/textures/pavement_albedo.png pavement_albedo.png COPYONLY)
# configure_file(assets/textures/rock_wall_16_diff_8k.jpg rock_wall_16_diff_8k.jpg COPYONLY)
# configure_file(assets/textures/rock_wall_16_nor_gl_8k.png rock_wall_16_nor_gl_8k.png COPYONLY)

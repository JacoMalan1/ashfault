#include <CLSTL/unique_ptr.h>
#include <ashfault/pipeline.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <ashfault/pipeline.h>
#include <ashfault/renderer.h>
#include <stdexcept>

void error_callback(int error, const char *desc) {
  std::printf("GLFW: %s\n", desc);
}

int main() {
  spdlog::set_level(spdlog::level::debug);

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);
  if (!window) {
    throw std::runtime_error("Failed to create GLFW window");
  }

  auto renderer = clstl::make_unique<ashfault::Renderer>();

  renderer->init(window);

  auto vshader = renderer->create_shader("shader.vert.spv");
  auto fshader = renderer->create_shader("shader.frag.spv");

  clstl::vector<VkVertexInputAttributeDescription> descriptions;
  VkVertexInputAttributeDescription pos{};
  pos.binding = 0;
  pos.location = 0;
  pos.offset = 0;
  pos.format = VK_FORMAT_R32G32B32_SFLOAT;
  descriptions.push_back(pos);

  auto pipeline =
      renderer->create_graphics_pipeline()
          .vertex_shader(vshader)
          .fragment_shader(fshader)
          .input_attribute_descriptions(descriptions, sizeof(float) * 3)
          .build();

  glfwTerminate();
}

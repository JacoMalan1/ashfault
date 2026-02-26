#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>
#include <ashfault/renderer.h>
#include <stdexcept>

void error_callback(int error, const char *desc) {
  std::printf("GLFW: %s\n", desc);
}

int main() {
  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow *window = glfwCreateWindow(800, 600, "Hello World!", nullptr, nullptr);
  if (!window) {
    throw std::runtime_error("Failed to create GLFW window");
  }

  auto renderer = std::make_shared<ashfault::Renderer>();

  renderer->init(window);

  glfwTerminate();
}

#include "GLFW/glfw3.h"
#include <ashfault/af_window.h>

namespace ashfault {
Window::Window(std::uint32_t width, std::uint32_t height) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  auto *monitor = glfwGetPrimaryMonitor();
  GLFWwindow *window =
      glfwCreateWindow(width, height, "AshFault", monitor, nullptr);
  this->m_Handle = window;
}

Window::~Window() {
  glfwDestroyWindow(this->m_Handle);
  glfwTerminate();
}

WindowDims Window::current_size() const {
  int width, height;
  glfwGetFramebufferSize(this->m_Handle, &width, &height);
  return {static_cast<std::uint32_t>(width),
          static_cast<std::uint32_t>(height)};
}
} // namespace ashfault

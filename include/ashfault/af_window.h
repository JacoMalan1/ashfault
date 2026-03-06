#ifndef ASHFAULT_WINDOW_H
#define GLFW_INCLUDE_VULKAN
#define ASHFAULT_WINDOW_H

#include <GLFW/glfw3.h>
#include <cstdint>

namespace ashfault {
struct WindowDims {
  std::uint32_t width, height;
};

class Window {
public:
  Window(std::uint32_t width, std::uint32_t height);
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  ~Window();

  WindowDims current_size() const;

private:
  GLFWwindow *m_Handle;
};
}

#endif

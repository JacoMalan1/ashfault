#ifndef ASHFAULT_WINDOW_H
#define ASHFAULT_WINDOW_H

#include <CLSTL/vector.h>
#include <cstdint>
#include <functional>
#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ashfault {
struct WindowDims {
  std::uint32_t width, height;
};

class Window {
public:
  Window(std::uint32_t width, std::uint32_t height, bool fullscreen = true);
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;
  ~Window();

  WindowDims current_size() const;
  clstl::vector<const char *> required_instance_extensions();
  void wait_events();
  void poll_events();
  VkSurfaceKHR create_surface(VkInstance instance);
  GLFWwindow *handle();
  bool should_close();

  void set_resize_callback(std::function<void(Window &, WindowDims)> callback);
	  
private:
  GLFWwindow *m_Handle;
  std::optional<std::function<void(Window&, WindowDims)>> m_ResizeCallback;
};
} // namespace ashfault

#endif

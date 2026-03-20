#ifndef ASHFAULT_WINDOW_H
#define ASHFAULT_WINDOW_H

#include <ashfault/ashfault.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ashfault {
struct ASHFAULT_API WindowDims {
  std::uint32_t width, height;
};

class ASHFAULT_API Window {
 public:
  Window(std::uint32_t width, std::uint32_t height, bool fullscreen = true);
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
  ~Window();

  WindowDims current_size() const;
  std::vector<const char*> required_instance_extensions();
  void wait_events();
  void poll_events();
  VkSurfaceKHR create_surface(VkInstance instance);
  GLFWwindow* handle();
  bool should_close();

  void set_resize_callback(std::function<void(Window&, WindowDims)> callback);
  void set_key_callback(
      std::function<void(Window&, int, int, int, int)> callback);

 private:
  void attach_pointer();

  GLFWwindow* m_Handle;
  std::optional<std::function<void(Window&, WindowDims)>> m_ResizeCallback;
  std::optional<std::function<void(Window&, int, int, int, int)>> m_KeyCallback;
};
}  // namespace ashfault

#endif

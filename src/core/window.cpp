#include <ashfault/core/window.h>
#include <ashfault/renderer/vkrenderer.h>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace ashfault {
Window::Window(std::uint32_t width, std::uint32_t height, bool fullscreen) {
  glfwInit();
  auto platform = glfwGetPlatform();
  if (platform == GLFW_PLATFORM_X11) {
    SPDLOG_WARN("GLFW Platform is X11");
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  auto *monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
  GLFWwindow *window =
      glfwCreateWindow(width, height, "AshFault", monitor, nullptr);
  if (!window) throw std::runtime_error("Failed to create GLFW window");
  this->m_Handle = window;
  glfwSetWindowUserPointer(window, nullptr);
}

Window::~Window() {
  glfwSetWindowUserPointer(this->m_Handle, nullptr);
  glfwDestroyWindow(this->m_Handle);
  glfwTerminate();
}

WindowDims Window::current_size() const {
  int width, height;
  glfwGetFramebufferSize(this->m_Handle, &width, &height);
  return {static_cast<std::uint32_t>(width),
          static_cast<std::uint32_t>(height)};
}

std::vector<const char *> Window::required_instance_extensions() {
  std::uint32_t count;
  const char **extensions = glfwGetRequiredInstanceExtensions(&count);

  std::vector<const char *> ret;
  ret.resize(count);
  for (std::size_t i = 0; i < ret.size(); i++) {
    ret[i] = extensions[i];
  }

  return ret;
}

void Window::wait_events() { glfwWaitEvents(); }

VkSurfaceKHR Window::create_surface(VkInstance instance) {
  VkSurfaceKHR ret;
  VK_CHECK_RESULT(
      glfwCreateWindowSurface(instance, this->m_Handle, nullptr, &ret));
  return ret;
}

GLFWwindow *Window::handle() { return this->m_Handle; }

bool Window::should_close() {
  return static_cast<bool>(glfwWindowShouldClose(this->m_Handle));
}

void Window::poll_events() { glfwPollEvents(); }

void Window::set_resize_callback(
    std::function<void(Window &, WindowDims)> callback) {
  this->m_ResizeCallback = callback;
  if (glfwGetWindowUserPointer(this->m_Handle) == nullptr) {
    this->attach_pointer();
  }

  glfwSetFramebufferSizeCallback(
      this->m_Handle, [](GLFWwindow *window, int, int) {
        Window *window_ptr =
            reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        if (!window_ptr) return;

        if (window_ptr->m_ResizeCallback.has_value()) {
          window_ptr->m_ResizeCallback.value()(*window_ptr,
                                               window_ptr->current_size());
        }
      });
}

void Window::set_key_callback(
    std::function<void(Window &, int, int, int, int)> callback) {
  if (glfwGetWindowUserPointer(this->m_Handle) == nullptr) {
    this->attach_pointer();
  }

  glfwSetKeyCallback(this->m_Handle, [](GLFWwindow *window, int key,
                                        int scancode, int action, int mods) {
    Window *window_ptr =
        reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    if (!window_ptr) return;
    if (window_ptr->m_KeyCallback.has_value()) {
      window_ptr->m_KeyCallback.value()(*window_ptr, key, scancode, action,
                                        mods);
    }
  });
}

void Window::attach_pointer() {
  glfwSetWindowUserPointer(this->m_Handle, this);
}
}  // namespace ashfault

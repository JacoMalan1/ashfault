#ifndef ASHFAULT_RENDERER_H
#define ASHFAULT_RENDERER_H
#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <cstdint>
#include <optional>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace ashfault {
class Renderer {
public:
  Renderer() = default;
  Renderer(const Renderer &);
  Renderer &operator=(const Renderer &) = delete;
  ~Renderer();

  void init(GLFWwindow *window);

private:
  VkInstance m_Instance;
  VkPhysicalDevice m_PhysicalDevice;
  VkDevice m_Device;
  VmaAllocator m_Allocator;

  void create_instance();
  void create_device();
  void create_allocator();
};

struct QueueSuitability {
  std::optional<std::uint32_t> graphics_queue;
  std::optional<std::uint32_t> transfer_queue;
};
} // namespace ashfault

#endif

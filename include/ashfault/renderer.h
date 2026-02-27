#ifndef ASHFAULT_RENDERER_H
#define ASHFAULT_RENDERER_H
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <optional>
#include <vk_mem_alloc.h>
#include <vector>
#include <CLSTL/vector.h>

namespace ashfault {
struct QueueSuitability {
  std::optional<std::uint32_t> graphics_queue;
  std::optional<std::uint32_t> present_queue;

  bool complete() const;
};

struct SwapchainSupportDetails {
  clstl::vector<VkPresentModeKHR> present_modes;
  clstl::vector<VkSurfaceFormatKHR> formats;
  VkSurfaceCapabilitiesKHR capabilities;
};

class Renderer {
public:
  Renderer() = default;
  Renderer(const Renderer &);
  Renderer &operator=(const Renderer &) = delete;
  ~Renderer();

  void init(GLFWwindow *window);

private:
  const std::vector<const char *> s_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  GLFWwindow *m_Window;
  VkInstance m_Instance;
  VkPhysicalDevice m_PhysicalDevice;
  VkDevice m_Device;
  VmaAllocator m_Allocator;
  VkSurfaceKHR m_Surface;
  VkSwapchainKHR m_Swapchain;
  VkSurfaceFormatKHR m_SurfaceFormat;
  VkPresentModeKHR m_PresentMode;
  VkExtent2D m_SwapExtent;
  clstl::vector<VkImage> m_SwapchainImages;
  clstl::vector<VkImageView> m_ImageViews;

  void create_instance();
  void create_surface();
  void create_device();
  void create_allocator();
  void setup_swapchain();

  VkSurfaceFormatKHR select_surface_format(const clstl::vector<VkSurfaceFormatKHR>&);
  VkPresentModeKHR select_present_mode(const clstl::vector<VkPresentModeKHR>&);
  VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR caps);

  bool check_device_suitability(VkPhysicalDevice device);
  QueueSuitability find_queue_families(VkPhysicalDevice device);
  bool check_device_extension_support(VkPhysicalDevice device);
  std::optional<std::pair<VkPhysicalDevice, QueueSuitability>> choose_physical_device();
  SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice device);
};
} // namespace ashfault

#endif

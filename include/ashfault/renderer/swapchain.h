#ifndef ASHFAULT_RENDERER_SWAPCHAIN_H
#define ASHFAULT_RENDERER_SWAPCHAIN_H

#include <CLSTL/vector.h>
#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <ashfault/renderer/renderer.h>

namespace ashfault {
class Swapchain {
public:
  Swapchain(VkSurfaceFormatKHR format, VkPresentModeKHR present_mode,
            std::uint32_t image_count, VkExtent2D extent, VkSurfaceKHR surface,
            SwapchainSupportDetails support, VkDevice device);
  Swapchain(const Swapchain &) = delete;
  Swapchain &operator=(const Swapchain &) = delete;

  std::uint32_t image_count() const;
  VkImage image(std::size_t index);
  VkImageView image_view(std::size_t index);

  void cleanup();

  void present(VkQueue queue, const clstl::vector<VkSemaphore> &wait_semaphores, std::uint32_t image_index);

  std::optional<std::uint32_t> acquire_image(VkSemaphore semaphore);
  void build();

private:
  std::uint32_t m_ImageCount;
  VkSwapchainKHR m_Handle;
  clstl::vector<VkImage> m_Images;
  clstl::vector<VkImageView> m_ImageViews;
  VkExtent2D m_SwapExtent;
  VkSurfaceFormatKHR m_SurfaceFormat;
  VkPresentModeKHR m_PresentMode;
  VkSurfaceKHR m_Surface;
  SwapchainSupportDetails m_Support;
  VkDevice m_Device;
};
} // namespace ashfault

#endif

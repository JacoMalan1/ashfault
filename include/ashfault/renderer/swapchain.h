#ifndef ASHFAULT_RENDERER_SWAPCHAIN_H
#define ASHFAULT_RENDERER_SWAPCHAIN_H

#include <ashfault/renderer/vkrenderer.h>
#include <cstdint>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <ashfault/ashfault.h>

namespace ashfault {
class ASHFAULT_API Swapchain {
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

  void present(VkQueue queue, const std::vector<VkSemaphore> &wait_semaphores,
               std::uint32_t image_index);
  VkSurfaceFormatKHR surface_format();
  VkExtent2D &swap_extent();

  std::optional<std::uint32_t> acquire_image(VkSemaphore semaphore);
  void build(VkExtent2D swap_extent);

private:
  std::uint32_t m_ImageCount;
  VkSwapchainKHR m_Handle;
  std::vector<VkImage> m_Images;
  std::vector<VkImageView> m_ImageViews;
  VkExtent2D m_SwapExtent;
  VkSurfaceFormatKHR m_SurfaceFormat;
  VkPresentModeKHR m_PresentMode;
  VkSurfaceKHR m_Surface;
  SwapchainSupportDetails m_Support;
  VkDevice m_Device;
};
} // namespace ashfault

#endif

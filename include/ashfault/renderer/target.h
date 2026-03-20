#ifndef ASHFAULT_RENDER_TARGET_H
#define ASHFAULT_RENDER_TARGET_H

#include <ashfault/ashfault.h>
#include <ashfault/renderer/vkrenderer.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace ashfault {
class ASHFAULT_API RenderTarget {
 public:
  RenderTarget(
      std::shared_ptr<VulkanRenderer> renderer,
      const std::optional<std::pair<VkImage, VmaAllocation>> &depth_image,
      std::optional<VkImageView> depth_view, const std::vector<VkImage> &images,
      const std::vector<VkImageView> &image_views,
      const std::optional<std::vector<VmaAllocation>> &allocations,
      const std::vector<VkCommandBuffer> &command_buffers,
      VkRect2D render_area);
  ~RenderTarget();

  VkCommandBuffer &command_buffer(std::uint32_t index);
  VkImage image(std::uint32_t index);
  VkImageView image_view(std::uint32_t index);
  void begin_rendering(std::uint32_t image_index, std::uint32_t current_frame);
  void end_rendering(std::uint32_t image_index, std::uint32_t current_frame,
                     bool present_source);

  void update_images(
      const std::vector<VkImage> &images,
      const std::vector<VkImageView> &image_views,
      const std::optional<std::vector<VmaAllocation>> &allocations);
  void update_depth_image(std::pair<VkImage, VmaAllocation> image,
                          VkImageView view);

 private:
  std::shared_ptr<VulkanRenderer> m_Renderer;
  std::vector<VkImage> m_Images;
  std::vector<VkImageView> m_ImageViews;
  std::optional<std::vector<VmaAllocation>> m_ImageAllocations;
  std::vector<VkCommandBuffer> m_CommandBuffers;
  std::optional<std::pair<VkImage, VmaAllocation>> m_DepthImage;
  std::optional<VkImageView> m_DepthView;
  VkRect2D m_RenderArea;
};
}  // namespace ashfault

#endif

#include <ashfault/renderer/target.h>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace ashfault {
RenderTarget::RenderTarget(
    std::shared_ptr<VulkanRenderer> renderer,
    const std::optional<std::pair<VkImage, VmaAllocation>> &depth_image,
    std::optional<VkImageView> depth_view, const std::vector<VkImage> &images,
    const std::vector<VkImageView> &image_views,
    const std::optional<std::vector<VmaAllocation>> &allocations,
    const std::vector<VkCommandBuffer> &command_buffers)
    : m_Renderer(renderer), m_Images(images), m_ImageViews(image_views),
      m_ImageAllocations(allocations), m_CommandBuffers(command_buffers),
      m_DepthImage(depth_image), m_DepthView(depth_view) {}

RenderTarget::~RenderTarget() {
  for (auto &view : m_ImageViews) {
    vkDestroyImageView(m_Renderer->device(), view, nullptr);
  }

  if (m_DepthImage.has_value()) {
    vkDestroyImageView(m_Renderer->device(), m_DepthView.value(), nullptr);
    vmaDestroyImage(m_Renderer->allocator(), m_DepthImage->first,
                    m_DepthImage->second);
  }

  if (m_ImageAllocations.has_value()) {
    for (std::size_t i = 0; i < m_Images.size(); i++) {
      vmaDestroyImage(m_Renderer->allocator(), m_Images[i],
                      m_ImageAllocations.value()[i]);
    }
  }
}

VkCommandBuffer RenderTarget::command_buffer(std::uint32_t idx) {
  return this->m_CommandBuffers[idx];
}

VkImage RenderTarget::image(std::uint32_t idx) { return this->m_Images[idx]; }

VkImageView RenderTarget::image_view(std::uint32_t idx) {
  return this->m_ImageViews[idx];
}
} // namespace ashfault

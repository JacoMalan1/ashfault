#include <ashfault/renderer/target.h>
#include <vulkan/vulkan_core.h>

#include <memory>

#include <ashfault/renderer/buffer.hpp>

namespace ashfault {
RenderTarget::RenderTarget(
    std::shared_ptr<VulkanRenderer> renderer,
    const std::optional<std::pair<VkImage, VmaAllocation>> &depth_image,
    std::optional<VkImageView> depth_view, const std::vector<VkImage> &images,
    const std::vector<VkImageView> &image_views,
    const std::optional<std::vector<VmaAllocation>> &allocations,
    const std::vector<VkCommandBuffer> &command_buffers, VkRect2D render_area)
    : m_Renderer(renderer),
      m_Images(images),
      m_ImageViews(image_views),
      m_ImageAllocations(allocations),
      m_CommandBuffers(command_buffers),
      m_DepthImage(depth_image),
      m_DepthView(depth_view),
      m_RenderArea(render_area) {}

void RenderTarget::destroy() {
  vkDeviceWaitIdle(m_Renderer->device());
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

    for (auto &view : m_ImageViews) {
      vkDestroyImageView(m_Renderer->device(), view, nullptr);
    }
  }

  vkFreeCommandBuffers(m_Renderer->device(), m_Renderer->command_pool(),
                       m_CommandBuffers.size(), m_CommandBuffers.data());
}

VkCommandBuffer &RenderTarget::command_buffer(std::uint32_t idx) {
  return this->m_CommandBuffers[idx];
}

VkImage RenderTarget::image(std::uint32_t idx) { return this->m_Images[idx]; }

VkImageView RenderTarget::image_view(std::uint32_t idx) {
  return this->m_ImageViews[idx];
}

void RenderTarget::begin_rendering(std::uint32_t image_index,
                                   std::uint32_t current_frame) {
  VkCommandBuffer cmd = command_buffer(current_frame);

  VkRenderingAttachmentInfo color_attachment{};
  color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  color_attachment.clearValue.color = {{0.03f, 0.03f, 0.03f, 1.0f}};
  color_attachment.imageView = image_view(image_index);
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkRenderingAttachmentInfo depth_attachment{};

  VkRenderingInfo rendering_info{};
  rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  rendering_info.colorAttachmentCount = 1;
  rendering_info.pColorAttachments = &color_attachment;
  rendering_info.renderArea = m_RenderArea;
  rendering_info.layerCount = 1;

  if (m_DepthImage.has_value()) {
    depth_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depth_attachment.clearValue.depthStencil = {1.0f, 1};
    depth_attachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_attachment.imageView = m_DepthView.value();
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    rendering_info.pDepthAttachment = &depth_attachment;
  }

  VkImageMemoryBarrier color_barrier{};
  color_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  color_barrier.image = image(image_index);
  color_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  color_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  color_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  color_barrier.subresourceRange.baseArrayLayer = 0;
  color_barrier.subresourceRange.baseMipLevel = 0;
  color_barrier.subresourceRange.layerCount = 1;
  color_barrier.subresourceRange.levelCount = 1;

  vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                       nullptr, 0, nullptr, 1, &color_barrier);

  if (m_DepthImage.has_value()) {
    VkImageMemoryBarrier depth_barrier{};
    depth_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    depth_barrier.image = m_DepthImage.value().first;
    depth_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                  VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    depth_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depth_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_barrier.subresourceRange.baseArrayLayer = 0;
    depth_barrier.subresourceRange.baseMipLevel = 0;
    depth_barrier.subresourceRange.layerCount = 1;
    depth_barrier.subresourceRange.levelCount = 1;

    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &depth_barrier);
  }

  vkCmdBeginRendering(cmd, &rendering_info);

  VkViewport viewport{};
  viewport.x = m_RenderArea.offset.x;
  viewport.y = m_RenderArea.offset.y;
  viewport.width = m_RenderArea.extent.width;
  viewport.height = m_RenderArea.extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(cmd, 0, 1, &viewport);
  vkCmdSetScissor(cmd, 0, 1, &m_RenderArea);
}

void RenderTarget::end_rendering(std::uint32_t image_index,
                                 std::uint32_t current_frame,
                                 bool present_source) {
  VkCommandBuffer cmd = command_buffer(current_frame);
  vkCmdEndRendering(cmd);

  VkImageMemoryBarrier color_barrier{};
  color_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  color_barrier.image = image(image_index);
  if (present_source) {
    color_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  } else {
    color_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  }
  color_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  color_barrier.newLayout = present_source
                                ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                                : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  color_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  color_barrier.subresourceRange.baseArrayLayer = 0;
  color_barrier.subresourceRange.baseMipLevel = 0;
  color_barrier.subresourceRange.layerCount = 1;
  color_barrier.subresourceRange.levelCount = 1;

  if (present_source) {
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0,
                         nullptr, 0, nullptr, 1, &color_barrier);
  } else {
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &color_barrier);
  }
}

void RenderTarget::update_images(
    const std::vector<VkImage> &images,
    const std::vector<VkImageView> &image_views,
    const std::optional<std::vector<VmaAllocation>> &allocations) {
  vkDeviceWaitIdle(m_Renderer->device());
  if (m_ImageAllocations.has_value()) {
    for (std::size_t i = 0; i < m_Images.size(); i++) {
      vmaDestroyImage(m_Renderer->allocator(), m_Images[i],
                      m_ImageAllocations.value()[i]);
      vkDestroyImageView(m_Renderer->device(), m_ImageViews[i], nullptr);
    }
  }

  m_Images = images;
  m_ImageViews = image_views;
  m_ImageAllocations = allocations;
}

void RenderTarget::update_depth_image(std::pair<VkImage, VmaAllocation> image,
                                      VkImageView view) {
  if (m_DepthImage.has_value()) {
    vmaDestroyImage(m_Renderer->allocator(), m_DepthImage->first,
                    m_DepthImage->second);
    vkDestroyImageView(m_Renderer->device(), m_DepthView.value(), nullptr);
  }

  m_DepthImage = image;
  m_DepthView = view;
}
}  // namespace ashfault

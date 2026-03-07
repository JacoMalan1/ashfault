#ifndef ASHFAULT_FRAME_H
#define ASHFAULT_FRAME_H

#include <ashfault/renderer/buffer.hpp>
#include <ashfault/renderer/pipeline.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace ashfault {
class Swapchain;

struct FrameData {
  Swapchain *swapchain;
  std::vector<VkSemaphore> render_finished_semaphores,
      image_available_semaphores;
  std::vector<VkFence> in_flight_fences;
  std::uint32_t current_frame;
  std::uint32_t image_index;
  VkQueue graphics_queue, present_queue;
};

class Renderer;

class VulkanAttachmentBuilder {
public:
  friend class VulkanRenderingAttachments;

  VulkanAttachmentBuilder &target(VkImageView view, VkImageLayout layout);
  VulkanAttachmentBuilder &resolve_target(VkImageView view,
                                          VkImageLayout layout);
  VulkanAttachmentBuilder &clear_color(float r, float g, float b, float a);
  VulkanAttachmentBuilder &clear_depth(float depth);

private:
  VulkanAttachmentBuilder(VkRenderingAttachmentInfo *dst_attachment);

  VkRenderingAttachmentInfo *m_DstAttachment;
};

class VulkanRenderingAttachments {
public:
  friend class Frame;
  VulkanRenderingAttachments();

  VulkanAttachmentBuilder build_color_attachment();
  VulkanAttachmentBuilder build_depth_attachment();

private:
  clstl::vector<VkRenderingAttachmentInfo> m_ColorInfos;
  VkRenderingAttachmentInfo m_DepthInfo;
};

class Frame {
public:
  Frame(FrameData frame_data);

  std::uint32_t image_index();
  std::uint32_t current_frame();
  VkQueue &graphics_queue();

  void begin_command_buffer(VkCommandBuffer cmd);
  void bind_graphics_pipeline(VkCommandBuffer cmd, GraphicsPipeline *pipeline);

  void insert_pipeline_barrier(VkCommandBuffer cmd, VkImage image,
                               VkImageAspectFlagBits aspects,
                               VkImageLayout old_layout,
                               VkImageLayout new_layout,
                               VkAccessFlags src_access_mask,
                               VkAccessFlags dst_access_mask,
                               VkPipelineStageFlags src_pipeline_stage,
                               VkPipelineStageFlags dst_pipeline_stage);
  void begin_rendering(VkCommandBuffer cmd,
                       const VulkanRenderingAttachments &attachments,
                       VkRect2D rendering_area);
  void end_rendering(VkCommandBuffer cmd);

  template <class V>
  void draw(VkCommandBuffer cmd, clstl::shared_ptr<VulkanBuffer<V>> buffer) {
    vkCmdBindVertexBuffers(cmd, 0, 1, &buffer->handle(), 0);
    vkCmdDraw(cmd, buffer->count(), 1, 0, 0);
  }

  void add_command_buffer_for_submit(
      VkCommandBuffer *cmd, const clstl::vector<VkSemaphore> &signal_semaphores,
      const clstl::vector<VkSemaphore> &wait_semaphores,
      VkPipelineStageFlags *wait_stages);

  void submit_all(VkFence fence);

  void wait_and_present();
  void set_viewport(VkCommandBuffer cmd, VkViewport viewport);
  void set_scissor(VkCommandBuffer cmd, VkRect2D viewport);

  VkSemaphore render_finished_semaphore();
  VkSemaphore image_available_semaphore();
  VkFence in_flight_fence();

private:
  FrameData m_FrameData;
  clstl::vector<VkSubmitInfo> m_Submits;
};
} // namespace ashfault

#endif

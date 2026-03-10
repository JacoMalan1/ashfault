#include <ashfault/renderer/frame.h>
#include <ashfault/renderer/pipeline.h>
#include <ashfault/renderer/renderer.h>
#include <ashfault/renderer/swapchain.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
Frame::Frame(FrameData data) : m_FrameData(data) {}

void Frame::begin_command_buffer(VkCommandBuffer cmd) {
  VK_CHECK_RESULT(vkResetCommandBuffer(cmd, 0));
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &begin_info));
}

void Frame::bind_graphics_pipeline(VkCommandBuffer cmd,
                                   GraphicsPipeline *pipeline) {
  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());
}

void Frame::wait_and_present() {
  clstl::vector<VkSemaphore> wait_semaphores;
  wait_semaphores.push_back(
      this->m_FrameData
          .render_finished_semaphores[this->m_FrameData.image_index]);
  this->m_FrameData.swapchain->present(this->m_FrameData.present_queue,
                                       wait_semaphores,
                                       this->m_FrameData.image_index);
}

VulkanRenderingAttachments::VulkanRenderingAttachments() {}

VulkanAttachmentBuilder VulkanRenderingAttachments::build_color_attachment() {
  VkRenderingAttachmentInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

  this->m_ColorInfos.push_back(info);
  return VulkanAttachmentBuilder(&this->m_ColorInfos.back());
}

VulkanAttachmentBuilder VulkanRenderingAttachments::build_depth_attachment() {
  VkRenderingAttachmentInfo info{};
  info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

  this->m_DepthInfo = info;
  return VulkanAttachmentBuilder(&this->m_DepthInfo.value());
}

VulkanAttachmentBuilder::VulkanAttachmentBuilder(
    VkRenderingAttachmentInfo *info)
    : m_DstAttachment(info) {}

VulkanAttachmentBuilder &VulkanAttachmentBuilder::target(VkImageView view,
                                                         VkImageLayout layout) {
  this->m_DstAttachment->imageView = view;
  this->m_DstAttachment->imageLayout = layout;
  return *this;
}

VulkanAttachmentBuilder &
VulkanAttachmentBuilder::resolve_target(VkImageView view,
                                        VkImageLayout layout) {
  this->m_DstAttachment->resolveImageView = view;
  this->m_DstAttachment->resolveImageLayout = layout;
  this->m_DstAttachment->resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
  return *this;
}

VulkanAttachmentBuilder &
VulkanAttachmentBuilder::clear_color(float r, float g, float b, float a) {
  this->m_DstAttachment->clearValue.color = {{r, g, b, a}};
  return *this;
}

VulkanAttachmentBuilder &VulkanAttachmentBuilder::clear_depth(float depth) {
  this->m_DstAttachment->clearValue.depthStencil = {depth, 1};
  return *this;
}

void Frame::insert_pipeline_barrier(VkCommandBuffer cmd, VkImage image,
                                    VkImageAspectFlagBits aspects,
                                    VkImageLayout old_layout,
                                    VkImageLayout new_layout,
                                    VkAccessFlags src_access_mask,
                                    VkAccessFlags dst_access_mask,
                                    VkPipelineStageFlags src_pipeline_stage,
                                    VkPipelineStageFlags dst_pipeline_stage) {
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.subresourceRange.aspectMask = aspects;
  barrier.image = image;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.oldLayout = old_layout;
  barrier.newLayout = new_layout;
  barrier.srcAccessMask = src_access_mask;
  barrier.dstAccessMask = dst_access_mask;

  vkCmdPipelineBarrier(cmd, src_pipeline_stage, dst_pipeline_stage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);
}

void Frame::begin_rendering(VkCommandBuffer cmd,
                            const VulkanRenderingAttachments &attachments,
                            VkRect2D rendering_area) {
  VkRenderingInfo rendering_info{};
  rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  rendering_info.colorAttachmentCount = attachments.m_ColorInfos.size();
  rendering_info.pColorAttachments = attachments.m_ColorInfos.data();
  rendering_info.pDepthAttachment = attachments.m_DepthInfo.has_value()
                                        ? &attachments.m_DepthInfo.value()
                                        : nullptr;
  rendering_info.layerCount = 1;
  rendering_info.renderArea = rendering_area;

  vkCmdBeginRendering(cmd, &rendering_info);
}

void Frame::end_rendering(VkCommandBuffer cmd) { vkCmdEndRendering(cmd); }

void Frame::add_command_buffer_for_submit(
    VkCommandBuffer *cmd, const clstl::vector<VkSemaphore> &signal_semaphores,
    const clstl::vector<VkSemaphore> &wait_semaphores,
    VkPipelineStageFlags *wait_stages) {

  vkEndCommandBuffer(*cmd);

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = cmd;
  submit_info.signalSemaphoreCount = signal_semaphores.size();
  submit_info.pSignalSemaphores = signal_semaphores.data();
  submit_info.waitSemaphoreCount = wait_semaphores.size();
  submit_info.pWaitSemaphores = wait_semaphores.data();
  submit_info.pWaitDstStageMask = wait_stages;

  this->m_Submits.push_back(submit_info);
}
void Frame::submit_all(VkFence fence) {
  vkQueueSubmit(this->m_FrameData.graphics_queue, this->m_Submits.size(),
                this->m_Submits.data(), fence);
}

std::uint32_t Frame::image_index() { return this->m_FrameData.image_index; }

VkSemaphore Frame::render_finished_semaphore() {
  return this->m_FrameData
      .render_finished_semaphores[this->m_FrameData.image_index];
}
VkSemaphore Frame::image_available_semaphore() {
  return this->m_FrameData
      .image_available_semaphores[this->m_FrameData.current_frame];
}

VkFence Frame::in_flight_fence() {
  return this->m_FrameData.in_flight_fences[this->m_FrameData.current_frame];
}

void Frame::set_viewport(VkCommandBuffer cmd, VkViewport viewport) {
  vkCmdSetViewport(cmd, 0, 1, &viewport);
}

void Frame::set_scissor(VkCommandBuffer cmd, VkRect2D scissor) {
  vkCmdSetScissor(cmd, 0, 1, &scissor);
}

std::uint32_t Frame::current_frame() { return this->m_FrameData.current_frame; }

VkQueue &Frame::graphics_queue() { return this->m_FrameData.graphics_queue; }

void Frame::bind_descriptor_set(VkCommandBuffer cmd, VulkanDescriptorSet *dset,
                                GraphicsPipeline *pipeline) {
  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipeline->layout(), 0, 1, &dset->handle(), 0,
                          nullptr);
}
} // namespace ashfault

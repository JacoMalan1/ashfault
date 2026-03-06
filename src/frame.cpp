#include "CLSTL/array.h"
#include "ashfault/pipeline.h"
#include "ashfault/renderer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <ashfault/frame.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
Frame::Frame(VkDevice device, VkCommandBuffer cmd, VkQueue submit_queue,
             VkQueue present_queue, VkSwapchainKHR swapchain, VkImage image,
             VkImage color_image, std::uint32_t image_i,
             std::uint32_t *current_frame, VkSemaphore image_available,
             VkSemaphore render_finished, VkFence in_flight, Renderer *renderer)
    : m_Device(device), m_CommandBuffer(cmd), m_Image(image),
      m_ColorImage(color_image), m_ImageAvailable(image_available),
      m_RenderFinished(render_finished), m_InFlight(in_flight),
      m_SubmitQueue(submit_queue), m_PresentQueue(present_queue),
      m_Swapchain(swapchain), m_Renderer(renderer), m_ImageIndex(image_i) {}

void Frame::bind_pipeline(const GraphicsPipeline *pipeline) {
  vkCmdBindPipeline(this->m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline->handle());
}

void Frame::submit() {
  VkImageMemoryBarrier image_barrier{};
  image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  image_barrier.image = this->m_Image;
  image_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  image_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  image_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  image_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  image_barrier.subresourceRange.baseArrayLayer = 0;
  image_barrier.subresourceRange.layerCount = 1;
  image_barrier.subresourceRange.baseMipLevel = 0;
  image_barrier.subresourceRange.levelCount = 1;

  clstl::array<VkImageMemoryBarrier, 1> barriers = {image_barrier};

  vkCmdEndRendering(this->m_CommandBuffer);
  
  vkEndCommandBuffer(this->m_CommandBuffer);

  auto imgui_cmd = m_Renderer->m_ImGuiCommandBuffers[m_Renderer->m_CurrentFrame];
  vkResetCommandBuffer(imgui_cmd, 0);
  VkCommandBufferBeginInfo imgui_cmd_info{};
  imgui_cmd_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  vkBeginCommandBuffer(imgui_cmd, &imgui_cmd_info);

  VkRenderingAttachmentInfo color_attachment{};
  color_attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  color_attachment.clearValue.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  color_attachment.imageView = m_Renderer->m_ImageViews[this->m_ImageIndex];

  VkRenderingInfo render_info{};
  render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  render_info.colorAttachmentCount = 1;
  render_info.pColorAttachments = &color_attachment;
  render_info.renderArea.extent = m_Renderer->m_SwapExtent;
  render_info.renderArea.offset.x = 0;
  render_info.renderArea.offset.y = 0;
  render_info.layerCount = 1;

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  barrier.image = m_Renderer->m_ViewportImages[m_ImageIndex].first;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.layerCount = 1;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.baseMipLevel = 0;

  vkCmdPipelineBarrier(imgui_cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0,
                       nullptr, 1, &barrier);
  vkCmdBeginRendering(imgui_cmd, &render_info);

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::DockSpaceOverViewport();
  for (auto f : this->m_UiCallbacks) {
    f();
  }
  ImGui::Begin("Viewport");
  auto size = ImGui::GetContentRegionAvail();
  m_Renderer->m_ViewportSize[0] = static_cast<std::uint32_t>(size.x);
  m_Renderer->m_ViewportSize[1] = static_cast<std::uint32_t>(size.y);
  ImGui::Image(m_Renderer->m_ImGuiViewportTextures[m_ImageIndex], size);
  ImGui::End();
  ImGui::Render();

  ImDrawData *draw_data = ImGui::GetDrawData();
  ImGui_ImplVulkan_RenderDrawData(draw_data, imgui_cmd);
  vkCmdEndRendering(imgui_cmd);
  vkCmdPipelineBarrier(imgui_cmd,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                       nullptr, barriers.size(), barriers.data());
  vkEndCommandBuffer(imgui_cmd);

  VkPipelineStageFlags wait_stages =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  clstl::array<VkCommandBuffer, 2> submits = {m_CommandBuffer, imgui_cmd};
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 2;
  submit_info.pCommandBuffers = submits.data();
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &this->m_ImageAvailable;
  submit_info.pWaitDstStageMask = &wait_stages;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &this->m_RenderFinished;

  vkQueueSubmit(this->m_SubmitQueue, 1, &submit_info, this->m_InFlight);

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &this->m_RenderFinished;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &this->m_Swapchain;
  present_info.pImageIndices = &this->m_ImageIndex;

  VkResult result = vkQueuePresentKHR(this->m_PresentQueue, &present_info);
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      this->m_Renderer->m_Resized) {
    this->m_Renderer->m_Resized = false;
    this->m_Renderer->recreate_swapchain();
  }

  if (this->m_CurrentFrame) {
    this->m_Renderer->m_CurrentFrame =
        (this->m_Renderer->m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
  }
}

void Frame::bind_descriptor_set(const VulkanDescriptorSet *descriptor,
                                const GraphicsPipeline *pipeline) {
  vkCmdBindDescriptorSets(this->m_CommandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout(),
                          0, 1, &descriptor->handle(), 0, nullptr);
}

void Frame::draw_ui(std::function<void(void)> func) {
  this->m_UiCallbacks.push_back(func);
}
} // namespace ashfault

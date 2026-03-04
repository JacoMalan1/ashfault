#include "CLSTL/array.h"
#include "ashfault/pipeline.h"
#include "ashfault/renderer.h"
#include <ashfault/frame.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
Frame::Frame(VkDevice device, VkCommandBuffer cmd, VkQueue submit_queue,
             VkQueue present_queue, VkSwapchainKHR swapchain, VkImage image, VkImage color_image,
             std::uint32_t image_i, std::uint32_t *current_frame,
             VkSemaphore image_available, VkSemaphore render_finished,
             VkFence in_flight, Renderer *renderer)
    : m_Device(device), m_CommandBuffer(cmd), m_Image(image), m_ColorImage(color_image),
      m_ImageAvailable(image_available), m_RenderFinished(render_finished),
      m_InFlight(in_flight), m_SubmitQueue(submit_queue),
      m_PresentQueue(present_queue), m_Swapchain(swapchain),
      m_Renderer(renderer), m_ImageIndex(image_i) {}

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
  vkCmdPipelineBarrier(this->m_CommandBuffer,
                       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                       VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
                       nullptr, barriers.size(), barriers.data());
  vkEndCommandBuffer(this->m_CommandBuffer);

  VkPipelineStageFlags wait_stages =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &this->m_CommandBuffer;
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

void Frame::bind_descriptor_set(const VulkanDescriptorSet *descriptor, const GraphicsPipeline *pipeline) {
  vkCmdBindDescriptorSets(this->m_CommandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->layout(),
                          0, 1, &descriptor->handle(), 0, nullptr);
}
} // namespace ashfault

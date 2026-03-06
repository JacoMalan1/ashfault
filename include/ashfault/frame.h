#ifndef ASHFAULT_FRAME_H
#define ASHFAULT_FRAME_H

#include <ashfault/buffer.hpp>
#include <ashfault/pipeline.h>
#include <functional>
#include <type_traits>
#include <vulkan/vulkan_core.h>

namespace ashfault {
class Renderer;

class Frame {
public:
  friend class Renderer;

  /// @brief Binds a graphics pipeline using `vkCmdBindPipeline`.
  void bind_pipeline(const GraphicsPipeline *pipeline);

  /// @brief Binds a descriptor set using `vkCmdBindDescriptorSets`.
  void bind_descriptor_set(const VulkanDescriptorSet *descriptor,
                           const GraphicsPipeline *pipeline);

  /// @brief Draws to the screen using `vkCmdDrawIndexed`.
  template <class V, class I>
  void draw_indexed(VulkanBuffer<V> *vertices, VulkanBuffer<I> *indices) {
    static_assert(std::is_integral<I>::value && std::is_unsigned<I>::value);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(this->m_CommandBuffer, 0, 1, &vertices->handle(),
                           &offset);
    vkCmdBindIndexBuffer(this->m_CommandBuffer, indices->handle(), 0,
                         index_type<I>::value);
    vkCmdDrawIndexed(this->m_CommandBuffer, indices->count(), 1, 0, 0, 0);
  }

  /// @brief Draws to the screen using `vkCmdDraw`.
  template <class V>
  void draw(VulkanBuffer<V> *vertices) {
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(this->m_CommandBuffer, 0, 1, &vertices->handle(),
                           &offset);
    vkCmdDraw(this->m_CommandBuffer, vertices->count(), 1, 0, 0);
  }

  void draw_ui(std::function<void(void)> func);

  /// @brief Ends the underlying command buffer and presents to the screen.
  ///
  /// @warning After this function has run, this frame object should not be used
  /// again.
  void submit();

private:
  Frame(VkDevice device, VkCommandBuffer cmd, VkQueue submit_queue,
        VkQueue present_queue, VkSwapchainKHR swapchain, VkImage image, VkImage color_image,
        std::uint32_t image_i, std::uint32_t *current_frame,
        VkSemaphore image_available, VkSemaphore render_finished,
        VkFence in_flight, Renderer *renderer);

  VkDevice m_Device;
  VkCommandBuffer m_CommandBuffer;
  VkImage m_Image;
  VkImage m_ColorImage;
  VkSemaphore m_ImageAvailable, m_RenderFinished;
  VkFence m_InFlight;
  VkQueue m_SubmitQueue, m_PresentQueue;
  VkSwapchainKHR m_Swapchain;
  Renderer *m_Renderer;
  std::uint32_t *m_CurrentFrame, m_ImageIndex;
  std::vector<std::function<void(void)>> m_UiCallbacks;
};
} // namespace ashfault

#endif

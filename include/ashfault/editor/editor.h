#ifndef ASHFAULT_EDITOR_H
#define ASHFAULT_EDITOR_H

#include <CLSTL/shared_ptr.h>
#include <ashfault/application.h>
#include <ashfault/core/engine.h>
#include <ashfault/core/scene.h>
#include <utility>

namespace ashfault {
struct SubmitData {
  clstl::vector<VkSemaphore> wait_semaphores, signal_semaphores;
  VkPipelineStageFlags wait_stages;
};

class Editor : public Application {
public:
  Editor(clstl::shared_ptr<Engine> engine, clstl::shared_ptr<Window> window);
  ~Editor();
  void run() override;
  SubmitData render_viewport(Frame &frame, Scene &scene);
  SubmitData render_ui(Frame &frame);

private:
  void create_images();
  void clean_images();

  std::vector<std::pair<VkImage, VmaAllocation>> m_ViewportImages;
  std::vector<VkImageView> m_ViewportImageViews;
  std::vector<VkCommandBuffer> m_PrimaryCommandBuffers, m_UiCommandBuffers;
  VkSampler m_ViewportSampler;
  std::vector<VkDescriptorSet> m_ImGuiViewportTextures;
  std::array<std::uint32_t, 2> m_ViewportSize;
  std::array<std::uint32_t, 2> m_ViewportRenderSize;
  WindowDims m_CurrentWindowSize;
  bool m_ViewportResized;
};
} // namespace ashfault

#endif

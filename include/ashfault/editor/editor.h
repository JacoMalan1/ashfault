#ifndef ASHFAULT_EDITOR_H
#define ASHFAULT_EDITOR_H

#include <ashfault/core/pipeline_manager.h>
#include <CLSTL/shared_ptr.h>
#include <ashfault/application.h>
#include <ashfault/core/engine.h>
#include <ashfault/core/scene.h>
#include <ashfault/editor/camera.h>
#include <mutex>
#include <spdlog/details/log_msg.h>
#include <utility>

namespace ashfault::editor {
struct SubmitData {
  clstl::vector<VkSemaphore> wait_semaphores, signal_semaphores;
  VkPipelineStageFlags wait_stages;
};

struct UniformBufferObject {
  glm::mat4 projection;
  glm::mat4 view;
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
  void build_ui_skeleton();
  void build_pipelines();
  void update_camera();
  void create_descriptor_sets();

  std::shared_ptr<EditorCamera> m_Camera;
  std::unique_ptr<EditorCameraControls> m_CameraControls;
  std::vector<std::pair<VkImage, VmaAllocation>> m_ViewportImages;
  std::vector<VkImageView> m_ViewportImageViews;
  std::pair<VkImage, VmaAllocation> m_DepthImage;
  VkImageView m_DepthImageView;
  std::vector<VkCommandBuffer> m_PrimaryCommandBuffers, m_UiCommandBuffers;
  VkSampler m_ViewportSampler;
  std::vector<VkDescriptorSet> m_ImGuiViewportTextures;
  std::array<std::uint32_t, 2> m_ViewportSize;
  std::array<std::uint32_t, 2> m_ViewportRenderSize;
  WindowDims m_CurrentWindowSize;
  bool m_ViewportResized;
  std::mutex m_LogsLock;
  std::vector<std::pair<spdlog::details::log_msg, std::string>> m_Logs;
  clstl::shared_ptr<VulkanDescriptorSet> m_DescriptorSet;
  clstl::shared_ptr<VulkanDescriptorPool> m_DescriptorPool;
  clstl::unique_ptr<PipelineManager> m_PipelineManager;
  clstl::shared_ptr<VulkanBuffer> m_UniformBuffer;
  std::pair<VkImage, VmaAllocation> m_ColorImage;
  VkImageView m_ColorImageView;
};
} // namespace ashfault::editor

#endif

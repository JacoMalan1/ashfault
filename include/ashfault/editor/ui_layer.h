#ifndef ASHFAULT_EDITOR_UI_LAYER_H
#define ASHFAULT_EDITOR_UI_LAYER_H

#include <ashfault/ashfault.h>
#include <ashfault/core/event.h>
#include <ashfault/core/layer.h>
#include <ashfault/editor/state.h>
#include <ashfault/renderer/target.h>
#include <imgui.h>

#include <memory>

#include <ashfault/core/asset_manager.hpp>
#include <ashfault/core/layer_stack.h>
#include <ashfault/editor/context.h>
#include <spdlog/common.h>

namespace ashfault {
class ASHFAULT_API EditorUiLayer : public Layer {
public:
  EditorUiLayer(EditorContext *context,
                std::shared_ptr<AssetManager> asset_manager);
  ~EditorUiLayer();

  void on_attach(LayerStack *layer_stack) override;
  void on_detach() override;
  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;
  void on_imgui_render() override;

private:
  void recreate_textures();
  void build_ui_skeleton();
  void render_scene_window();
  void render_console_window();
  void render_component_window();
  void render_file_browser();
  void render_toolbar();

  std::shared_ptr<RenderTarget> m_ViewportTarget;
  std::vector<
      std::pair<std::shared_ptr<RenderTarget>, std::vector<VkDescriptorSet>>>
      m_TexturePreviewTargets;
  VkSampler m_ViewportSampler;
  std::vector<VkDescriptorSet> m_ViewportTextures;
  std::optional<ImVec2> m_PreviousViewportSize;
  bool m_UpdateViewport;
  LayerStack *m_LayerStack;
  EditorContext *m_EditorContext;
  std::shared_ptr<AssetManager> m_AssetManager;
  State::RuntimeState m_RuntimeState;
  std::vector<std::pair<spdlog::level::level_enum, std::string>> m_LogLines;
};
}  // namespace ashfault

#endif

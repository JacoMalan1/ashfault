#ifndef ASHFAULT_EDITOR_UI_LAYER_H
#define ASHFAULT_EDITOR_UI_LAYER_H

#include "ashfault/core/layer_stack.h"
#include <ashfault/renderer/target.h>
#include <ashfault/core/event.h>
#include <ashfault/core/layer.h>
#include <memory>
#include <imgui.h>

namespace ashfault {
class EditorUiLayer : public Layer {
public:
  EditorUiLayer();
  ~EditorUiLayer();

  void on_attach(LayerStack *layer_stack) override;
  void on_detach() override;
  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;
  void on_imgui_render() override;

private:
  void recreate_textures();

  std::shared_ptr<RenderTarget> m_ViewportTarget;
  VkSampler m_ViewportSampler;
  std::vector<VkDescriptorSet> m_ViewportTextures;
  std::optional<ImVec2> m_PreviousViewportSize;
  bool m_UpdateViewport;
  LayerStack *m_LayerStack;
};
} // namespace ashfault

#endif

#ifndef ASHFAULT_EDITOR_LAYER_H
#define ASHFAULT_EDITOR_LAYER_H

#include <ashfault/core/camera.h>
#include <ashfault/core/layer.h>
#include <ashfault/core/layer_stack.h>
#include <ashfault/core/mesh.h>
#include <ashfault/core/scene.h>
#include <ashfault/editor/context.h>
#include <ashfault/editor/state.h>

#include <memory>

#include <ashfault/core/asset_manager.hpp>

namespace ashfault {
class ASHFAULT_API EditorLayer : public Layer {
public:
  EditorLayer(EditorContext *context,
              std::shared_ptr<AssetManager> asset_manager);
  ~EditorLayer();

  void on_attach(LayerStack *layer_stack) override;
  void on_detach() override;

  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;

private:
  std::shared_ptr<PerspectiveCamera> m_PerspectiveCamera;
  std::shared_ptr<OrthoCamera> m_OrthoCamera;
  std::unique_ptr<Scene> m_ActiveScene;
  EditorContext *m_Context;
  std::shared_ptr<AssetManager> m_AssetManager;
  State::RuntimeState m_RuntimeState;
};
}  // namespace ashfault

#endif

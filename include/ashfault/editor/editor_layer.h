#ifndef ASHFAULT_EDITOR_LAYER_H
#define ASHFAULT_EDITOR_LAYER_H

#include <ashfault/core/camera.h>
#include <ashfault/core/layer.h>
#include <ashfault/core/mesh.h>

#include <memory>

#include "ashfault/core/layer_stack.h"
#include "ashfault/core/scene.h"

namespace ashfault {
class EditorLayer : public Layer {
 public:
  EditorLayer();
  ~EditorLayer();

  void on_attach(LayerStack *layer_stack) override;
  void on_detach() override;

  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;

 private:
  std::shared_ptr<Mesh> m_Mesh;
  std::shared_ptr<PerspectiveCamera> m_PerspectiveCamera;
  std::shared_ptr<OrthoCamera> m_OrthoCamera;
  std::shared_ptr<Scene> m_ActiveScene;
};
}  // namespace ashfault

#endif

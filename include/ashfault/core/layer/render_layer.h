#ifndef ASHFAULT_RENDER_LAYER_H
#define ASHFAULT_RENDER_LAYER_H

#include <ashfault/core/layer.h>
#include <ashfault/renderer/renderer.h>
namespace ashfault {
class RenderLayer : public Layer {
public:
  RenderLayer(Renderer &renderer);

  void on_attach() override;
  void on_detach() override;
  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;

private:
  Renderer &m_Renderer;
};
}

#endif

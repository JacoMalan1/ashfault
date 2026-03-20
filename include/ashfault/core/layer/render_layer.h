#ifndef ASHFAULT_RENDER_LAYER_H
#define ASHFAULT_RENDER_LAYER_H

#include "ashfault/core/layer_stack.h"
#include <ashfault/core/layer.h>
#include <ashfault/renderer/vkrenderer.h>
#include <ashfault/ashfault.h>

namespace ashfault {
class ASHFAULT_API RenderLayer : public Layer {
public:
  RenderLayer();

  void on_attach(LayerStack *) override;
  void on_detach() override;
  void on_update(float dt) override;
  void on_render() override;
  void on_event(Event &event) override;
};
}

#endif

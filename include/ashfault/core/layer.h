#ifndef ASHFAULT_LAYER_H
#define ASHFAULT_LAYER_H

#include <ashfault/ashfault.h>
#include <ashfault/core/event.h>

namespace ashfault {
class LayerStack;

class ASHFAULT_API Layer {
public:
  Layer();
  virtual ~Layer() = default;

  virtual void on_attach(LayerStack *layer_stack) {}
  virtual void on_detach() {}

  virtual void on_update(float dt) {}
  virtual void on_render() {}
  virtual void on_event(Event &event) {}
  virtual void on_imgui_render() {}

  bool is_enabled() const;
  void set_enabled(bool enabled);

protected:
  bool m_Enabled;
  LayerStack *m_LayerStack;
};
}  // namespace ashfault

#endif

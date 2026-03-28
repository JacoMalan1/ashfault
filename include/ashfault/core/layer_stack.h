#ifndef ASHFAULT_LAYER_STACK_H
#define ASHFAULT_LAYER_STACK_H

#include <ashfault/ashfault.h>
#include <ashfault/core/layer.h>

#include <cstddef>
#include <vector>

namespace ashfault {
class ASHFAULT_API LayerStack {
public:
  LayerStack();
  ~LayerStack();

  void push_layer(Layer *layer);
  void push_overlay(Layer *layer);

  void pop_layer(Layer *layer);
  void pop_overlay(Layer *layer);

  void on_update(float dt);
  void on_render();
  void on_event(Event &event);
  void on_imgui_render();

private:
  std::vector<Layer *> m_Layers;
  std::size_t m_InsertPosition;
};
}  // namespace ashfault

#endif

#ifndef ASHFAULT_LAYER_STACK_H
#define ASHFAULT_LAYER_STACK_H

#include <ashfault/core/layer.h>
#include <ashfault/ashfault.h>
#include <vector>

namespace ashfault {
class ASHFAULT_API LayerStack {
public:
  LayerStack();
  ~LayerStack();

  void push_layer(Layer *layer);
  void push_overlay(Layer *layer);

  void pop_layer(Layer *layer);

  void on_update(float dt);
  void on_render();
  void on_event(Event &event);

private:
  std::vector<Layer *> m_Layers;
  std::vector<Layer *>::iterator m_InsertPosition;
};
}

#endif

#include <algorithm>
#include <ashfault/core/layer_stack.h>

namespace ashfault {
LayerStack::LayerStack() : m_Layers(), m_InsertPosition(m_Layers.begin()) {}
LayerStack::~LayerStack() {
  for (auto *layer : m_Layers) {
    delete layer;
  }
}

void LayerStack::push_layer(Layer *layer) {
  this->m_Layers.insert(m_InsertPosition++, layer);
}

void LayerStack::push_overlay(Layer *layer) {
  this->m_Layers.push_back(layer);
}

void LayerStack::pop_layer(Layer *layer) {
  for (auto it = m_Layers.begin(); it != m_Layers.end(); it++) {
    if (*it == layer) {
      m_Layers.erase(it);
      return;
    }
  }
}

void LayerStack::on_update(float dt) {
  std::for_each(m_Layers.begin(), m_Layers.end(), [=](Layer *layer) {
    if (layer->is_enabled())
      layer->on_update(dt);
  });
}

void LayerStack::on_render() {
  std::for_each(m_Layers.begin(), m_Layers.end(), [=](Layer *layer) {
    if (layer->is_enabled())
      layer->on_render();
  });
}

void LayerStack::on_event(Event &event) {
  std::for_each(m_Layers.rbegin(), m_Layers.rend(), [&](Layer *layer) {
    if (layer->is_enabled())
      layer->on_event(event);
  });
}
} // namespace ashfault

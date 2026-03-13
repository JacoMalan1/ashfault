#include <algorithm>
#include <ashfault/core/layer_stack.h>

namespace ashfault {
LayerStack::LayerStack() : m_Layers(), m_InsertPosition(0) {}
LayerStack::~LayerStack() {
  for (auto *layer : m_Layers) {
    layer->on_detach();
    delete layer;
  }
}

void LayerStack::push_layer(Layer *layer) {
  this->m_Layers.insert(m_Layers.begin() + m_InsertPosition, layer);
  m_InsertPosition++;
  layer->on_attach();
}

void LayerStack::push_overlay(Layer *layer) {
  this->m_Layers.push_back(layer);
  layer->on_attach();
}

void LayerStack::pop_layer(Layer *layer) {
  for (auto it = m_Layers.begin(); it != m_Layers.begin() + m_InsertPosition; it++) {
    if (*it == layer) {
      layer->on_detach();
      m_Layers.erase(it);
      m_InsertPosition--;
      return;
    }
  }
}

void LayerStack::pop_overlay(Layer *layer) {
  for (auto it = m_Layers.begin() + m_InsertPosition; it != m_Layers.end(); it++) {
    if (*it == layer) {
      layer->on_detach();
      m_Layers.erase(it);
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

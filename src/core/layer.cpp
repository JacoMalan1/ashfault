#include <ashfault/core/layer.h>

namespace ashfault {
Layer::Layer() : m_Enabled(true) {}

void Layer::set_enabled(bool enabled) {
  m_Enabled = enabled;
}

bool Layer::is_enabled() const {
  return m_Enabled;
}
}

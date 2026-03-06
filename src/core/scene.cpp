#include <ashfault/core/registry.hpp>
#include <ashfault/core/scene.h>

namespace ashfault {
Scene::Scene() : m_NextEntityId(0), m_ComponentRegistry(), m_Entities() {}

Entity Scene::create_entity() {
  Entity e(m_NextEntityId++);
  return e;
}

const ComponentRegistry &Scene::component_registry() const {
  return this->m_ComponentRegistry;
}

ComponentRegistry &Scene::component_registry() {
  return this->m_ComponentRegistry;
}
} // namespace ashfault

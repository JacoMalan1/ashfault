#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/scene.h>
#include <vulkan/vulkan_core.h>

#include <ashfault/core/registry.hpp>
#include <ashfault/renderer/buffer.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace ashfault {
Scene::Scene() : m_NextEntityId(0), m_ComponentRegistry(), m_Entities() {}

Entity Scene::create_entity() {
  Entity e(m_NextEntityId++);
  this->m_Entities.push_back(e);
  return e;
}

const ComponentRegistry& Scene::component_registry() const {
  return this->m_ComponentRegistry;
}

ComponentRegistry& Scene::component_registry() {
  return this->m_ComponentRegistry;
}
}  // namespace ashfault

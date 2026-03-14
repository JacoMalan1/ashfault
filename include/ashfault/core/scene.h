#ifndef ASHFAULT_SCENE_H
#define ASHFAULT_SCENE_H

#include <ashfault/core/entity.h>
#include <ashfault/core/registry.hpp>
#include <ashfault/renderer/frame.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <ashfault/ashfault.h>

namespace ashfault {
class ASHFAULT_API Scene {
public:
  Scene();

  Entity create_entity();
  ComponentRegistry &component_registry();
  const ComponentRegistry &component_registry() const;

private:
  Entity::id_type m_NextEntityId;
  ComponentRegistry m_ComponentRegistry;
  std::vector<Entity> m_Entities;
};
} // namespace ashfault

#endif

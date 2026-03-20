#ifndef ASHFAULT_SCENE_H
#define ASHFAULT_SCENE_H

#include <ashfault/ashfault.h>
#include <ashfault/core/entity.h>
#include <ashfault/core/mesh.h>
#include <vulkan/vulkan.h>

#include <ashfault/core/registry.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace ashfault {
class ASHFAULT_API Scene {
 public:
  Scene();

  Entity create_entity();
  ComponentRegistry &component_registry();
  const ComponentRegistry &component_registry() const;

  void draw_all();

 private:
  Entity::id_type m_NextEntityId;
  ComponentRegistry m_ComponentRegistry;
  std::vector<Entity> m_Entities;
};
}  // namespace ashfault

#endif

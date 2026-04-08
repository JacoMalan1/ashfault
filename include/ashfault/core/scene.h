#ifndef ASHFAULT_SCENE_H
#define ASHFAULT_SCENE_H

#include <ashfault/ashfault.h>
#include <ashfault/core/entity.h>
#include <ashfault/core/mesh.h>
#include <vulkan/vulkan.h>

#include <ashfault/core/registry.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <ashfault/renderer/descriptor_set.h>

namespace ashfault {
namespace serialization {
class SceneSerializer;
}

class ASHFAULT_API Scene {
public:
  friend class serialization::SceneSerializer;
  Scene();

  Entity create_entity();
  void delete_entity(Entity e);
  ComponentRegistry &component_registry();
  const ComponentRegistry &component_registry() const;

  void draw_all();
  const std::vector<Entity> &entities() const;

  std::optional<Entity> get_entity(Entity::id_type id);

private:
  Entity::id_type m_NextEntityId;
  ComponentRegistry m_ComponentRegistry;
  std::vector<Entity> m_Entities;
};
}  // namespace ashfault

#endif

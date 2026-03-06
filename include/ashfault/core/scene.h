#ifndef ASHFAULT_SCENE_H
#define ASHFAULT_SCENE_H

#include <ashfault/core/registry.hpp>
#include <CLSTL/vector.h>
#include <ashfault/core/entity.h>

namespace ashfault {
class Scene {
public:
  Scene();

  Entity create_entity();
  ComponentRegistry &component_registry();
  const ComponentRegistry &component_registry() const;

private:
  Entity::id_type m_NextEntityId;
  ComponentRegistry m_ComponentRegistry;
  clstl::vector<Entity> m_Entities;
};
}

#endif

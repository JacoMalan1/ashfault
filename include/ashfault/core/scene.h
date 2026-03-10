#ifndef ASHFAULT_SCENE_H
#define ASHFAULT_SCENE_H

#include <CLSTL/vector.h>
#include <ashfault/core/engine.h>
#include <ashfault/core/entity.h>
#include <ashfault/core/registry.hpp>
#include <ashfault/renderer/frame.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace ashfault {
class Scene {
public:
  Scene();

  Entity create_entity();
  ComponentRegistry &component_registry();
  const ComponentRegistry &component_registry() const;

  void record_command_buffers(VkCommandBuffer cmd, Engine &engine,
                              Frame &frame);

private:
  Entity::id_type m_NextEntityId;
  ComponentRegistry m_ComponentRegistry;
  clstl::vector<Entity> m_Entities;
};
} // namespace ashfault

#endif

#include "ashfault/core/engine.h"
#include "ashfault/renderer/buffer.hpp"
#include <ashfault/core/component/mesh.h>
#include <ashfault/core/registry.hpp>
#include <ashfault/core/scene.h>
#include <vulkan/vulkan_core.h>

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

void Scene::record_command_buffers(VkCommandBuffer cmd, Engine &engine,
                                   Frame &frame) {
  for (auto e : this->m_Entities) {
    std::optional<MeshComponent<Vertex, std::uint16_t> *> mesh_component =
        this->m_ComponentRegistry
            .get_component<MeshComponent<Vertex, std::uint16_t>>(e);
    if (mesh_component.has_value()) {
      frame.bind_graphics_pipeline(
          cmd, engine.pipeline_manager().get_graphics_pipeline("simple"));
      vkCmdBindVertexBuffers(
          cmd, 0, 1, &mesh_component.value()->vertex_buffer->handle(), nullptr);
      vkCmdBindIndexBuffer(cmd, mesh_component.value()->index_buffer->handle(),
                           0, index_type<std::uint16_t>::value);
      vkCmdDrawIndexed(cmd, mesh_component.value()->index_buffer->count(), 1, 0,
                       0, 0);
    }
  }
}
} // namespace ashfault

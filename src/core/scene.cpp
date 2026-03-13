#include <ashfault/core/component/transform.h>
#include <ashfault/core/engine.h>
#include <ashfault/renderer/buffer.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <ashfault/core/component/mesh.h>
#include <ashfault/core/registry.hpp>
#include <ashfault/core/scene.h>
#include <vulkan/vulkan_core.h>

namespace ashfault {
Scene::Scene() : m_NextEntityId(0), m_ComponentRegistry(), m_Entities() {}

Entity Scene::create_entity() {
  Entity e(m_NextEntityId++);
  this->m_Entities.push_back(e);
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
    std::optional<MeshComponent *> mesh_component =
        this->m_ComponentRegistry.get_component<MeshComponent>(e);
    std::optional<TransformComponent *> transform =
        this->m_ComponentRegistry.get_component<TransformComponent>(e);
    if (mesh_component.has_value()) {
      auto pipeline = engine.pipeline_manager().get_graphics_pipeline("simple");
      frame.bind_graphics_pipeline(cmd, pipeline);

      glm::mat4 model = glm::identity<glm::mat4>();
      if (transform.has_value()) {
        glm::mat4 T = glm::translate(glm::identity<glm::mat4>(),
                                     transform.value()->position);
        glm::mat4 R = glm::rotate(glm::identity<glm::mat4>(),
                                  transform.value()->rotation.x,
                                  glm::vec3(1.0f, 0.0f, 0.0f));
        R = glm::rotate(R, transform.value()->rotation.y,
                        glm::vec3(0.0f, 1.0f, 0.0f));
        R = glm::rotate(R, transform.value()->rotation.z,
                        glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 S =
            glm::scale(glm::identity<glm::mat4>(), transform.value()->scale);

        model = T * R * S;
      }

      auto mesh = mesh_component.value()->mesh;
      VkDeviceSize offset = 0;
      vkCmdPushConstants(cmd, pipeline->layout(), VK_SHADER_STAGE_VERTEX_BIT, 0,
                         sizeof(glm::mat4), &model);
      vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->vertex_buffer()->handle(),
                             &offset);
      vkCmdDraw(cmd, mesh->vertex_buffer()->count(), 1, 0, 0);
    }
  }
}
} // namespace ashfault

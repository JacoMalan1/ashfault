#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/scene.h>
#include <vulkan/vulkan_core.h>

#include <ashfault/core/registry.hpp>
#include <ashfault/renderer/buffer.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ashfault/renderer/renderer.h"

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

void Scene::draw_all() {
  for (auto &entity : m_Entities) {
    auto mesh = m_ComponentRegistry.get_component<MeshComponent>(entity);
    auto transform =
        m_ComponentRegistry.get_component<TransformComponent>(entity);

    if (mesh.has_value()) {
      auto model_mat = glm::identity<glm::mat4>();
      if (transform.has_value()) {
        auto T = glm::translate(glm::identity<glm::mat4>(),
                                transform.value()->position);
        auto S =
            glm::scale(glm::identity<glm::mat4>(), transform.value()->scale);

        auto R = glm::rotate(glm::identity<glm::mat4>(),
                             transform.value()->rotation.x,
                             glm::vec3(1.0f, 0.0f, 0.0f));
        R = glm::rotate(R, transform.value()->rotation.y,
                        glm::vec3(0.0f, 1.0f, 0.0f));
        R = glm::rotate(R, transform.value()->rotation.z,
                        glm::vec3(0.0f, 0.0f, 1.0f));

        model_mat = T * R * S;
      }

      Renderer::submit_mesh(*mesh.value()->mesh, model_mat);
    }
  }
}
}  // namespace ashfault

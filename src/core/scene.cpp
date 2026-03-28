#include <ashfault/core/component/material.h>
#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/component/light.h>
#include <ashfault/core/material.h>
#include <ashfault/core/scene.h>
#include <ashfault/renderer/light.h>
#include <vulkan/vulkan_core.h>

#include <ashfault/core/registry.hpp>
#include <ashfault/renderer/buffer.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <optional>

#include <ashfault/renderer/renderer.h>

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
    auto directional_light =
        m_ComponentRegistry.get_component<DirectionalLightComponent>(entity);
    auto point_light =
        m_ComponentRegistry.get_component<PointLightComponent>(entity);
    auto transform =
        m_ComponentRegistry.get_component<TransformComponent>(entity);
    auto material_component =
        m_ComponentRegistry.get_component<MaterialComponent>(entity);

    if (directional_light.has_value()) {
      Light light{};
      light.direction = glm::vec4(directional_light.value()->direction, 0.0f);
      light.color = glm::vec4(directional_light.value()->color, 0.0f);
      light.position = glm::vec4(0);
      light.position.w = 1.0f;
      Renderer::add_light(light);
    }

    if (point_light.has_value()) {
      Light light{};
      light.position = glm::vec4(0.0f, 0.0f, 0.0f, 2.0f);
      light.color = glm::vec4(point_light.value()->color, 0.0f);
      if (transform.has_value()) {
        light.position += glm::vec4(transform.value()->position, 0.0f);
      }
      Renderer::add_light(light);
    }

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
      Material material = material_component
                              ? material_component.value()->material
                              : Material{.albedo_texture_index = 0};

      Renderer::submit_mesh(*mesh.value()->mesh.get(), model_mat, material);
    }
  }
}

const std::vector<Entity> &Scene::entities() const { return m_Entities; }

void Scene::delete_entity(Entity e) {
  for (auto it = m_Entities.begin(); it != m_Entities.end(); it++) {
    if (*it == e) {
      m_Entities.erase(it);
      m_ComponentRegistry.delete_entity(*it);
      return;
    }
  }
}

std::optional<Entity> Scene::get_entity(Entity::id_type id) {
  for (auto &e : m_Entities) {
    if (e.handle() == id) {
      return e;
    }
  }

  return std::optional<Entity>();
}
}  // namespace ashfault

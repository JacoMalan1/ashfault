#include <ashfault/core/component/light.h>
#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/script.h>
#include <ashfault/core/component/tag.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/entity.h>
#include <ashfault/core/material.h>
#include <ashfault/core/serialization/mesh_serializer.h>
#include <ashfault/core/serialization/light_serializer.h>
#include <ashfault/core/serialization/scene_serializer.h>
#include "scene_generated.h"
#include "tag_generated.h"
#include <ashfault/core/serialization/script_serializer.h>
#include <ashfault/core/serialization/transform_serializer.h>
#include <ashfault/core/scene.h>
#include <algorithm>
#include <ashfault/core/asset_manager.hpp>
#include <vector>

namespace ashfault::serialization {
flatbuffers::Offset<ashfault::serialization::Scene> SceneSerializer::serialize(
    flatbuffers::FlatBufferBuilder &builder, const ashfault::Scene &scene) {
  std::vector<flatbuffers::Offset<Entity>> entities;
  for (auto e : scene.entities()) {
    std::vector<flatbuffers::Offset<Component>> components{};
    auto transform =
        scene.component_registry().get_component<ashfault::TransformComponent>(
            e);
    auto tag =
        scene.component_registry().get_component<ashfault::TagComponent>(e);
    auto mesh =
        scene.component_registry().get_component<ashfault::MeshComponent>(e);
    auto point_light =
        scene.component_registry().get_component<ashfault::PointLightComponent>(
            e);
    auto directional_light =
        scene.component_registry()
            .get_component<ashfault::DirectionalLightComponent>(e);
    auto script =
        scene.component_registry().get_component<ashfault::ScriptComponent>(e);
    if (transform.has_value()) {
      auto offset = TransformSerializer::serialize(builder, *transform.value());
      auto component = CreateComponent(
          builder, ComponentData_TransformComponent, offset.Union());
      components.push_back(component);
    }
    if (tag.has_value()) {
      auto str = builder.CreateString(tag.value()->tag);
      auto offset = CreateTagComponent(builder, str);
      auto component =
          CreateComponent(builder, ComponentData_TagComponent, offset.Union());
      components.push_back(component);
    }
    if (mesh.has_value()) {
      auto offset = MeshSerializer::serialize(builder, *mesh.value());
      auto component =
          CreateComponent(builder, ComponentData_MeshComponent, offset.Union());
      components.push_back(component);
    }
    if (point_light.has_value()) {
      auto offset =
          PointLightSerializer::serialize(builder, *point_light.value());
      auto component = CreateComponent(
          builder, ComponentData_PointLightComponent, offset.Union());
      components.push_back(component);
    }
    if (directional_light.has_value()) {
      auto offset = DirectionalLightSerializer::serialize(
          builder, *directional_light.value());
      auto component = CreateComponent(
          builder, ComponentData_DirectionalLightComponent, offset.Union());
      components.push_back(component);
    }
    if (script.has_value()) {
      auto offset = ScriptSerializer::serialize(builder, *script.value());
      auto component = CreateComponent(builder, ComponentData_ScriptComponent,
                                       offset.Union());
      components.push_back(component);
    }
    auto entity = CreateEntityDirect(builder, e.handle(), &components);
    entities.push_back(entity);
  }
  return CreateSceneDirect(builder, &entities);
}

ashfault::Scene SceneSerializer::deserialize(const void *buffer,
                                             std::size_t len,
                                             AssetManager *asset_manager) {
  flatbuffers::Verifier verifier(reinterpret_cast<const std::uint8_t *>(buffer),
                                 len);
  VerifySceneBuffer(verifier);

  auto scene = GetScene(buffer);
  ashfault::Scene ret{};
  for (auto e : *scene->entities()) {
    ashfault::Entity entity(e->id());
    ret.m_Entities.push_back(entity);

    for (auto c : *e->components()) {
      switch (c->data_type()) {
        case ComponentData_TransformComponent: {
          auto transform_buff = c->data_as_TransformComponent();
          ashfault::TransformComponent comp{
              .position = glm::vec3(transform_buff->position()->x(),
                                    transform_buff->position()->y(),
                                    transform_buff->position()->z()),
              .rotation = glm::vec3(transform_buff->rotation()->x(),
                                    transform_buff->rotation()->y(),
                                    transform_buff->rotation()->z()),
              .scale = glm::vec3(transform_buff->scale()->x(),
                                 transform_buff->scale()->y(),
                                 transform_buff->scale()->z()),
          };
          ret.component_registry().add_component(entity, comp);
          break;
        }
        case ComponentData_TagComponent: {
          auto buff = c->data_as_TagComponent();
          ashfault::TagComponent comp{.tag = buff->tag()->str()};
          ret.component_registry().add_component(entity, comp);
          break;
        }
        case ComponentData_MeshComponent: {
          auto buff = c->data_as_MeshComponent();
          auto comp = MeshSerializer::deserialize(buff, asset_manager);
          ret.component_registry().add_component(entity, comp);
          break;
        }
        case ComponentData_PointLightComponent: {
          auto buff = c->data_as_PointLightComponent();
          auto comp = PointLightSerializer::deserialize(buff);
          ret.component_registry().add_component(entity, comp);
          break;
        }
        case ComponentData_DirectionalLightComponent: {
          auto buff = c->data_as_DirectionalLightComponent();
          auto comp = DirectionalLightSerializer::deserialize(buff);
          ret.component_registry().add_component(entity, comp);
          break;
        }
        case ComponentData_ScriptComponent: {
          auto buff = c->data_as_ScriptComponent();
          auto comp = ScriptSerializer::deserialize(buff, asset_manager);
          ret.component_registry().add_component(entity, comp);
          break;
        }
        case ComponentData_NONE: {
          break;
        }
      }
    }
  }

  if (ret.m_Entities.empty()) {
    ret.m_NextEntityId = 0;
  } else {
    ret.m_NextEntityId =
        std::max_element(ret.m_Entities.begin(), ret.m_Entities.end(),
                         [](ashfault::Entity a, ashfault::Entity b) {
                           return a.handle() < b.handle();
                         })
            ->handle() +
        1;
  }

  return ret;
}
}  // namespace ashfault::serialization

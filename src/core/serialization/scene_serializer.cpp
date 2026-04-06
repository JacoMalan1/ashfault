#include <ashfault/core/component/mesh.h>
#include <ashfault/core/component/tag.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/serialization/mesh_serializer.h>
#include <ashfault/core/serialization/scene_serializer.h>
#include "scene_generated.h"
#include "tag_generated.h"
#include <ashfault/core/serialization/transform_serializer.h>
#include <ashfault/core/scene.h>
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
    auto entity = CreateEntityDirect(builder, e.handle(), &components);
    entities.push_back(entity);
  }
  return CreateSceneDirect(builder, &entities);
}

ashfault::Scene SceneSerializer::deserialize(const void *buffer,
                                             std::size_t len,
                                             AssetManager *asset_manager) {
  flatbuffers::Verifier verifier(reinterpret_cast<const std::uint8_t*>(buffer), len);
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
        case ComponentData_NONE: {
          break;
        }
      }
    }
  }

  return ret;
}
}  // namespace ashfault::serialization

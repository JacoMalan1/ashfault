#include <ashfault/core/component/transform.h>
#include <ashfault/core/serialization/transform_serializer.h>
#include <flatbuffers/flatbuffer_builder.h>
#include <glm/ext/vector_float3.hpp>
#include "transform_generated.h"

namespace ashfault::serialization {
flatbuffers::Offset<TransformComponent> TransformSerializer::serialize(
    flatbuffers::FlatBufferBuilder &builder,
    const ashfault::TransformComponent &component) {
  Vec3 position =
      Vec3(component.position.x, component.position.y, component.position.z);
  Vec3 rotation =
      Vec3(component.rotation.x, component.rotation.y, component.rotation.z);
  Vec3 scale = Vec3(component.scale.x, component.scale.y, component.scale.z);
  return CreateTransformComponent(builder, &position, &rotation, &scale);
}

ashfault::TransformComponent TransformSerializer::deserialize(
    const void *buff) {
  auto c = GetTransformComponent(buff);
  return ashfault::TransformComponent{
      .position =
          glm::vec3(c->position()->x(), c->position()->y(), c->position()->z()),
      .rotation =
          glm::vec3(c->rotation()->x(), c->rotation()->y(), c->rotation()->z()),
      .scale = glm::vec3(c->scale()->x(), c->scale()->y(), c->scale()->z())};
}
}  // namespace ashfault::serialization

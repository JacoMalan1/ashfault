#include <ashfault/core/component/light.h>
#include <ashfault/core/serialization/light_serializer.h>
#include "light_generated.h"

namespace ashfault::serialization {
flatbuffers::Offset<PointLightComponent> PointLightSerializer::serialize(
    flatbuffers::FlatBufferBuilder &builder,
    const ashfault::PointLightComponent &light) {
  auto position = Vec3(light.position.x, light.position.y, light.position.z);
  auto color = Vec3(light.color.x, light.color.y, light.color.z);
  return CreatePointLightComponent(builder, &position, &color);
}

ashfault::PointLightComponent PointLightSerializer::deserialize(
    const PointLightComponent *light) {
  return ashfault::PointLightComponent{
      .position = glm::vec3(light->position()->x(), light->position()->y(),
                            light->position()->z()),
      .color = glm::vec3(light->color()->x(), light->color()->y(),
                         light->color()->z()),
  };
}

flatbuffers::Offset<DirectionalLightComponent>
DirectionalLightSerializer::serialize(
    flatbuffers::FlatBufferBuilder &builder,
    const ashfault::DirectionalLightComponent &light) {
  auto direction =
      Vec3(light.direction.x, light.direction.y, light.direction.z);
  auto color = Vec3(light.color.x, light.color.y, light.color.z);
  return CreateDirectionalLightComponent(builder, &direction, &color);
}

ashfault::DirectionalLightComponent DirectionalLightSerializer::deserialize(
    const DirectionalLightComponent *light) {
  return ashfault::DirectionalLightComponent{
      .direction = glm::vec3(light->direction()->x(), light->direction()->y(),
                             light->direction()->z()),
      .color = glm::vec3(light->color()->x(), light->color()->y(),
                         light->color()->z()),
  };
}
}  // namespace ashfault::serialization

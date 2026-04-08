#ifndef ASHFAULT_POINT_LIGHT_SERIALIZER_H
#define ASHFAULT_POINT_LIGHT_SERIALIZER_H

#include <ashfault/core/component/light.h>
#include <flatbuffers/buffer.h>
#include "light_generated.h"

namespace ashfault::serialization {
class PointLightSerializer {
public:
  static flatbuffers::Offset<PointLightComponent> serialize(
      flatbuffers::FlatBufferBuilder &builder,
      const ashfault::PointLightComponent &light);

  static ashfault::PointLightComponent deserialize(
      const PointLightComponent *light);
};

class DirectionalLightSerializer {
public:
  static flatbuffers::Offset<DirectionalLightComponent> serialize(
      flatbuffers::FlatBufferBuilder &builder,
      const ashfault::DirectionalLightComponent &light);

  static ashfault::DirectionalLightComponent deserialize(
      const DirectionalLightComponent *light);
};
}  // namespace ashfault::serialization

#endif

#ifndef ASHFAULT_TRANSFORM_SERIALIZER_H
#define ASHFAULT_TRANSFORM_SERIALIZER_H

#include <ashfault/core/component/transform.h>
#include <flatbuffers/flatbuffer_builder.h>
#include "transform_generated.h"

namespace ashfault::serialization {
class TransformSerializer {
public:
  static flatbuffers::Offset<TransformComponent> serialize(
      flatbuffers::FlatBufferBuilder &builder,
      const ashfault::TransformComponent &component);

  static ashfault::TransformComponent deserialize(const void *buff);
};
}  // namespace ashfault::serialization

#endif

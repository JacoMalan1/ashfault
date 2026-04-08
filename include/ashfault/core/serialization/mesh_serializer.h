#ifndef ASHFAULT_MESH_SERIALIZER_H
#define ASHFAULT_MESH_SERIALIZER_H

#include <ashfault/core/component/mesh.h>
#include <flatbuffers/flatbuffer_builder.h>
#include <ashfault/core/asset_manager.hpp>
#include "mesh_generated.h"

namespace ashfault::serialization {
class MeshSerializer {
public:
  static flatbuffers::Offset<ashfault::serialization::MeshComponent> serialize(
      flatbuffers::FlatBufferBuilder &builder,
      const ashfault::MeshComponent &mesh);

  static ashfault::MeshComponent deserialize(
      const ashfault::serialization::MeshComponent *mesh,
      AssetManager *asset_manager);
};
}  // namespace ashfault::serialization

#endif

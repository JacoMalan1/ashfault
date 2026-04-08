#ifndef ASHFAULT_SCENE_SERIALIZER_H
#define ASHFAULT_SCENE_SERIALIZER_H

#include <flatbuffers/flatbuffer_builder.h>
#include <ashfault/core/scene.h>
#include <ashfault/core/asset_manager.hpp>
#include "scene_generated.h"

namespace ashfault::serialization {
class SceneSerializer {
public:
  static flatbuffers::Offset<ashfault::serialization::Scene> serialize(
      flatbuffers::FlatBufferBuilder &builder, const ashfault::Scene &scene);

  static ashfault::Scene deserialize(const void *scene, std::size_t len,
                                     AssetManager *asset_manager);
};
}  // namespace ashfault::serialization

#endif

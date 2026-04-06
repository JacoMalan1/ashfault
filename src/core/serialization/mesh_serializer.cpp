#include <ashfault/core/serialization/mesh_serializer.h>
#include <ashfault/core/asset_manager.hpp>
#include <stdexcept>
#include "asset_generated.h"
#include "mesh_generated.h"

namespace ashfault::serialization {
flatbuffers::Offset<ashfault::serialization::MeshComponent>
MeshSerializer::serialize(flatbuffers::FlatBufferBuilder &builder,
                          const ashfault::MeshComponent &mesh) {
  auto id = builder.CreateString(mesh.mesh.id());
  auto path = builder.CreateString(mesh.mesh.path());
  auto mesh_asset = CreateAsset(builder, id, path);
  return CreateMeshComponent(builder, mesh_asset);
}

ashfault::MeshComponent MeshSerializer::deserialize(
    const ashfault::serialization::MeshComponent *mesh,
    AssetManager *asset_manager) {
  if (!mesh) {
    throw std::runtime_error("Attempt to deserialize null mesh component");
  }
  auto mesh_asset = asset_manager->load<Mesh>(mesh->mesh()->id()->str(),
                                              mesh->mesh()->path()->str());
  return {.mesh = mesh_asset};
}
}  // namespace ashfault::serialization

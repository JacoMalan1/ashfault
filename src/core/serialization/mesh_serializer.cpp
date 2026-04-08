#include <ashfault/core/serialization/mesh_serializer.h>
#include <ashfault/core/texture.h>
#include <ashfault/renderer/renderer.h>
#include <flatbuffers/buffer.h>
#include <ashfault/core/asset_manager.hpp>
#include "asset_generated.h"
#include "mesh_generated.h"

namespace ashfault::serialization {
flatbuffers::Offset<ashfault::serialization::MeshComponent>
MeshSerializer::serialize(flatbuffers::FlatBufferBuilder &builder,
                          const ashfault::MeshComponent &mesh) {
  auto id = builder.CreateString(mesh.mesh.id());
  auto path = builder.CreateString(mesh.mesh.path());
  auto mesh_asset = CreateAsset(builder, id, path);
  flatbuffers::Offset<Material> material = 0;
  if (mesh.material.has_value()) {
    flatbuffers::Offset<Asset> albedo = 0;
    if (mesh.material->albedo_texture.has_value()) {
      albedo = CreateAsset(
          builder, builder.CreateString(mesh.material->albedo_texture->id()),
          builder.CreateString(mesh.material->albedo_texture->path()));
    }

    flatbuffers::Offset<Asset> normal = 0;
    if (mesh.material->normal_texture.has_value()) {
      normal = CreateAsset(
          builder, builder.CreateString(mesh.material->normal_texture->id()),
          builder.CreateString(mesh.material->normal_texture->path()));
    }

    material = CreateMaterial(builder, mesh.material->diffuse,
                              mesh.material->specular, albedo, normal);
  }
  return CreateMeshComponent(builder, mesh_asset, material);
}

ashfault::MeshComponent MeshSerializer::deserialize(
    const ashfault::serialization::MeshComponent *mesh,
    AssetManager *asset_manager) {
  auto mesh_asset = asset_manager->load<Mesh>(mesh->mesh()->id()->str(),
                                              mesh->mesh()->path()->str());
  std::optional<ashfault::Material> material{};
  if (mesh->material()) {
    std::optional<ashfault::Asset<ashfault::Texture>> albedo = {};
    if (mesh->material()->albedo_map()) {
      albedo = asset_manager->load<Texture>(
          mesh->material()->albedo_map()->id()->str(),
          mesh->material()->albedo_map()->path()->str());
    }

    std::optional<ashfault::Asset<ashfault::Texture>> normal = {};
    if (mesh->material()->normal_map()) {
      normal = asset_manager->load<Texture>(
          mesh->material()->normal_map()->id()->str(),
          mesh->material()->normal_map()->path()->str());
    }

    material = ashfault::Material{.diffuse = mesh->material()->diffuse(),
                                  .specular = mesh->material()->specular(),
                                  .albedo_texture = albedo,
                                  .normal_texture = normal};
  }
  return {.mesh = mesh_asset, .material = material};
}
}  // namespace ashfault::serialization

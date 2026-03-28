#include <ashfault/core/asset/mesh_loader.h>

#include <memory>

namespace ashfault {
MeshLoader::MeshLoader() : AssetLoader<Mesh>() {}

std::shared_ptr<Mesh> MeshLoader::read(const std::string &path) {
  return Mesh::load_from_file(path);
}
}  // namespace ashfault

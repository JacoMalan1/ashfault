#ifndef ASHFAULT_MESH_LOADER_H
#define ASHFAULT_MESH_LOADER_H

#include <ashfault/ashfault.h>
#include <ashfault/core/mesh.h>

#include <ashfault/core/asset_manager.hpp>

namespace ashfault {
class ASHFAULT_API MeshLoader : public AssetLoader<Mesh> {
public:
  MeshLoader();

protected:
  std::shared_ptr<Mesh> read(const std::string &path) override;
};
}  // namespace ashfault

#endif

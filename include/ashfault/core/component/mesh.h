#ifndef ASHFAULT_COMPONENT_MODEL_H
#define ASHFAULT_COMPONENT_MODEL_H

#include <ashfault/ashfault.h>
#include <ashfault/core/material.h>
#include <ashfault/core/mesh.h>

#include <ashfault/core/asset_manager.hpp>
#include <ashfault/renderer/buffer.hpp>

namespace ashfault {
struct ASHFAULT_API MeshComponent {
  Asset<Mesh> mesh;
  std::optional<Material> material;
};
}  // namespace ashfault

#endif

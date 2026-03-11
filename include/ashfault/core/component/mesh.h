#ifndef ASHFAULT_COMPONENT_MODEL_H
#define ASHFAULT_COMPONENT_MODEL_H

#include <ashfault/core/mesh.h>
#include <ashfault/renderer/buffer.hpp>
#include <memory>

namespace ashfault {
struct MeshComponent {
  std::shared_ptr<Mesh> mesh;
};
} // namespace ashfault

#endif

#ifndef ASHFAULT_COMPONENT_MODEL_H
#define ASHFAULT_COMPONENT_MODEL_H

#include <ashfault/core/mesh.h>
#include <CLSTL/shared_ptr.h>
#include <ashfault/renderer/buffer.hpp>

namespace ashfault {
struct MeshComponent {
  clstl::shared_ptr<Mesh> mesh;
};
} // namespace ashfault

#endif

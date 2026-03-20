#ifndef ASHFAULT_VERTEX_H
#define ASHFAULT_VERTEX_H

#include <ashfault/ashfault.h>

#include <glm/glm.hpp>

namespace ashfault {
struct ASHFAULT_API StaticVertex {
  glm::vec3 position;
  glm::vec3 normal;
};
}  // namespace ashfault

#endif

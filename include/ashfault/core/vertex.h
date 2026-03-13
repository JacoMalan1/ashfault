#ifndef ASHFAULT_VERTEX_H
#define ASHFAULT_VERTEX_H

#include <glm/glm.hpp>
#include <ashfault/ashfault.h>

namespace ashfault {
struct ASHFAULT_API StaticVertex {
  glm::vec3 position;
  glm::vec3 normal;
};
}

#endif

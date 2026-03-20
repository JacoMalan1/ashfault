#ifndef ASHFAULT_COMPONENT_TRANSFORM_H
#define ASHFAULT_COMPONENT_TRANSFORM_H

#include <ashfault/ashfault.h>

#include <glm/glm.hpp>

namespace ashfault {
struct ASHFAULT_API TransformComponent {
  glm::vec3 position, rotation, scale;
};
}  // namespace ashfault

#endif

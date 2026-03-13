#ifndef ASHFAULT_COMPONENT_TRANSFORM_H
#define ASHFAULT_COMPONENT_TRANSFORM_H

#include <glm/glm.hpp>
#include <ashfault/ashfault.h>

namespace ashfault {
struct ASHFAULT_API TransformComponent {
  glm::vec3 position, rotation, scale;
};
} // namespace ashfault

#endif

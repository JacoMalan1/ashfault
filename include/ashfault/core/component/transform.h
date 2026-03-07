#ifndef ASHFAULT_COMPONENT_TRANSFORM_H
#define ASHFAULT_COMPONENT_TRANSFORM_H

#include <glm/glm.hpp>

namespace ashfault {
struct TransformComponent {
  glm::vec3 position, rotation, scale;
};
} // namespace ashfault

#endif

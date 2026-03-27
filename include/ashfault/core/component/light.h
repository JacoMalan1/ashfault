#ifndef ASHFAULT_COMPONENT_LIGHT_H
#define ASHFAULT_COMPONENT_LIGHT_H

#include <glm/glm.hpp>

namespace ashfault {
struct DirectionalLightComponent {
  glm::vec3 direction;
  glm::vec3 color;
};

struct PointLightComponent {
  glm::vec3 position;
  glm::vec3 color;
};
}

#endif

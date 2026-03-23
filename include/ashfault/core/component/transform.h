#ifndef ASHFAULT_COMPONENT_TRANSFORM_H
#define ASHFAULT_COMPONENT_TRANSFORM_H

#include <ashfault/ashfault.h>

#include <glm/glm.hpp>
#include "glm/ext/matrix_transform.hpp"

namespace ashfault {
struct ASHFAULT_API TransformComponent {
  glm::vec3 position, rotation, scale;

  glm::mat4 calculate_matrix() {
    auto T =
        glm::translate(glm::identity<glm::mat4>(), position);
    auto S = glm::scale(glm::identity<glm::mat4>(), scale);

    auto R =
        glm::rotate(glm::identity<glm::mat4>(), rotation.x,
                    glm::vec3(1.0f, 0.0f, 0.0f));
    R = glm::rotate(R, rotation.y,
                    glm::vec3(0.0f, 1.0f, 0.0f));
    R = glm::rotate(R, rotation.z,
                    glm::vec3(0.0f, 0.0f, 1.0f));

    return T * R * S;
  }
};
}  // namespace ashfault

#endif

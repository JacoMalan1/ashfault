#ifndef ASHFAULT_RENDERER_LIGHT_H
#define ASHFAULT_RENDERER_LIGHT_H

#include <glm/ext/vector_float4.hpp>
namespace ashfault {
struct Light {
  glm::vec4 position;
  glm::vec4 direction;
  glm::vec4 color;
};
}  // namespace ashfault

#endif

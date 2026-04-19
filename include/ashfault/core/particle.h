#ifndef ASHFAULT_PARTICLE_H
#define ASHFAULT_PARTICLE_H

#include <glm/glm.hpp>

namespace ashfault {
struct Particle {
  glm::vec4 position;
  glm::vec4 velocity;
  float lifetime;
  float _padding[3];
};
}

#endif

#ifndef ASHFAULT_ENTITY_H
#define ASHFAULT_ENTITY_H

#include <ashfault/ashfault.h>

#include <cstdint>

namespace ashfault {
namespace serialization {
  class SceneSerializer;
}

class ASHFAULT_API Entity {
public:
  using id_type = std::uint64_t;
  friend class Scene;
  friend class ashfault::serialization::SceneSerializer;
  id_type handle() const;

  bool operator==(const Entity &other) const;
  bool operator!=(const Entity &other) const;

private:
  Entity(std::uint64_t handle);
  id_type m_Handle;
};
}  // namespace ashfault

#endif

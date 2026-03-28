#ifndef ASHFAULT_ENTITY_H
#define ASHFAULT_ENTITY_H

#include <ashfault/ashfault.h>

#include <cstdint>

namespace ashfault {
class ASHFAULT_API Entity {
public:
  using id_type = std::uint64_t;
  friend class Scene;
  id_type handle() const;

  bool operator==(const Entity &other) const;
  bool operator!=(const Entity &other) const;

private:
  Entity(std::uint64_t handle);
  id_type m_Handle;
};
}  // namespace ashfault

#endif

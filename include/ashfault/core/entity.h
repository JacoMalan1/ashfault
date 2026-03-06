#ifndef ASHFAULT_ENTITY_H
#define ASHFAULT_ENTITY_H

#include <cstdint>

namespace ashfault {
class Entity {
public:
  using id_type = std::uint64_t;
  friend class Scene;
  id_type handle() const;

private:
  Entity(std::uint64_t handle);
  id_type m_Handle;
};
}

#endif

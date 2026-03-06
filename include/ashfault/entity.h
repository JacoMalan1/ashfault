#ifndef ASHFAULT_ENTITY_H
#define ASHFAULT_ENTITY_H

#include <cstdint>

class Entity {
private:
  Entity(std::uint64_t handle);
  std::uint64_t m_Handle;
};

#endif

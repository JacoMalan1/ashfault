#include <ashfault/core/entity.h>

namespace ashfault {
Entity::Entity(Entity::id_type id) : m_Handle(id) {}

Entity::Entity::id_type Entity::handle() const {
  return this->m_Handle;
}
}

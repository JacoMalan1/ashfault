#ifndef ASHFAULT_REGISTRY_H
#define ASHFAULT_REGISTRY_H

#include <ashfault/core/entity.h>
#include <CLSTL/shared_ptr.h>
#include <memory>
#include <optional>
#include <typeindex>
#include <unordered_map>

namespace ashfault {
template <typename C> class ComponentPool {
public:
  ComponentPool() {}

  void add(Entity entity, const C &component) {
    m_Components[entity.handle()] = component;
  }

  std::optional<C *> get(Entity e) {
    if (!m_Components.count(e.handle())) {
      return {};
    }

    return &m_Components[e.handle()];
  }

  std::optional<const C *> get(Entity e) const {
    if (!m_Components.count(e.handle())) {
      return {};
    }

    return m_Components[e.handle()];
  }

private:
  std::unordered_map<Entity::id_type, C> m_Components;
};

class ComponentRegistry {
public:
  template <typename C> ComponentPool<C> &get_pool() {
    std::type_index index(typeid(C));
    if (!m_ComponentPools.count(index)) {
      auto pool = std::make_shared<ComponentPool<C>>();
      m_ComponentPools[index] = std::static_pointer_cast<void>(pool);
      return *pool;
    }

    return *std::static_pointer_cast<ComponentPool<C>>(m_ComponentPools[index]);
  }

  template <typename C> void add_component(Entity e, const C &component) {
    ComponentPool<C> &pool = this->get_pool<C>();
    pool.add(e, component);
  }

  template <typename C> std::optional<C *> get_component(Entity e) {
    ComponentPool<C> &pool = this->get_pool<C>();
    return pool.get(e);
  }

  template <typename C> std::optional<const C *> get_component(Entity e) const {
    ComponentPool<C> &pool = this->get_pool<C>();
    return pool.get(e);
  }

private:
  std::unordered_map<std::type_index, std::shared_ptr<void>> m_ComponentPools;
};
} // namespace ashfault

#endif

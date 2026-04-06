#ifndef ASHFAULT_REGISTRY_H
#define ASHFAULT_REGISTRY_H

#include <ashfault/core/entity.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <optional>
#include <typeindex>
#include <unordered_map>

#include <ashfault/ashfault.h>

namespace ashfault {
class ASHFAULT_API IComponentPool {
public:
  virtual ~IComponentPool() = default;
  virtual void remove(Entity e) = 0;
};

template <typename C>
class ASHFAULT_API ComponentPool : public IComponentPool {
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

    return &m_Components.at(e.handle());
  }

  void remove(Entity e) override {
    SPDLOG_INFO("Removing entity {} from registry {}", e.handle(),
                typeid(C).name());
    if (!m_Components.count(e.handle())) {
      return;
    }

    auto it = m_Components.find(e.handle());
    m_Components.erase(it);
  }

private:
  std::unordered_map<Entity::id_type, C> m_Components;
};

class ASHFAULT_API ComponentRegistry {
public:
  template <typename C>
  ComponentPool<C> &get_pool() {
    std::type_index index(typeid(C));
    if (!m_ComponentPools.count(index)) {
      auto pool = std::make_shared<ComponentPool<C>>();
      m_ComponentPools[index] = std::static_pointer_cast<IComponentPool>(pool);
      return *pool;
    }

    return *std::static_pointer_cast<ComponentPool<C>>(m_ComponentPools[index]);
  }

  template <typename C>
  const ComponentPool<C> &get_pool() const {
    std::type_index index(typeid(C));
    if (!m_ComponentPools.count(index)) {
      throw std::runtime_error("No component pool with specified type");
    }

    return *std::static_pointer_cast<ComponentPool<C>>(m_ComponentPools.at(index));
  }

  template <typename C>
  void add_component(Entity e, const C &component) {
    ComponentPool<C> &pool = this->get_pool<C>();
    pool.add(e, component);
  }

  template <typename C>
  std::optional<C *> get_component(Entity e) {
    ComponentPool<C> &pool = this->get_pool<C>();
    return pool.get(e);
  }

  template <typename C>
  std::optional<const C *> get_component(Entity e) const {
    const ComponentPool<C> &pool = this->get_pool<C>();
    return pool.get(e);
  }

  template <typename C>
  void remove_component(Entity e) {
    ComponentPool<C> &pool = this->get_pool<C>();
    pool.remove(e);
  }

  void delete_entity(Entity e) {
    for (auto x : m_ComponentPools) {
      x.second->remove(e);
    }
  }

private:
  std::unordered_map<std::type_index, std::shared_ptr<IComponentPool>>
      m_ComponentPools;
};
}  // namespace ashfault

#endif

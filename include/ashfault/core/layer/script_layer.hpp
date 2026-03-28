#ifndef ASHFAULT_LAYER_SCRIPT_H
#define ASHFAULT_LAYER_SCRIPT_H

#include <ashfault/ashfault.h>
#include <ashfault/core/layer.h>
#include <functional>
#include <memory>
#include <sol/object.hpp>
#include <sol/sol.hpp>
#include <ashfault/core/asset_manager.hpp>
#include <sol/types.hpp>
#include <unordered_map>
#include <ashfault/editor/context.h>
#include <ashfault/core/script.h>
#include <spdlog/logger.h>

namespace ashfault {
class ASHFAULT_API ScriptLayer : public Layer {
public:
  ScriptLayer(const ScriptLayer &) = delete;
  ScriptLayer(ScriptLayer &&) = default;
  ScriptLayer &operator=(const ScriptLayer &) = delete;
  ScriptLayer &operator=(ScriptLayer &&) = default;
  ScriptLayer(std::shared_ptr<AssetManager> asset_manager,
              RuntimeContext *context);

  void on_attach(LayerStack *layer_stack) override;
  void on_detach() override;

  void on_update(float dt) override;
  void on_event(Event &event) override;

  template <typename T>
  void register_ecs_component(const std::string &name) {
    sol::table type_table = m_LuaState[name];
    auto *key = type_table.pointer();
    m_EcsGetters.insert(std::make_pair(
        key,
        [](Scene *scene, sol::state_view lua,
           Entity::id_type e) -> sol::object {
          auto entity = scene->get_entity(e);
          if (!entity.has_value()) return sol::make_object(lua, sol::nil);
          auto component =
              scene->component_registry().get_component<T>(entity.value());
          return component ? sol::make_object(lua, std::ref(*component.value()))
                           : sol::make_object(lua, sol::nil);
        }));
    m_EcsAdders.emplace(key, [](Scene *scene, sol::state_view,
                                Entity::id_type e, sol::object component) {
      T c = component.as<T>();

      auto entity = scene->get_entity(e);
      if (!entity.has_value()) return;
      scene->component_registry().add_component(entity.value(), c);
    });
  }

private:
  void bind_engine_functions();

  sol::state m_LuaState;
  std::shared_ptr<AssetManager> m_AssetManager;
  RuntimeContext *m_Context;
  sol::table m_SceneTable;
  std::shared_ptr<spdlog::logger> m_ScriptLogger;
  std::unordered_map<const void *,
                     std::function<sol::object(Scene *scene, sol::state_view,
                                               Entity::id_type)>>
      m_EcsGetters;
  std::unordered_map<const void *,
                     std::function<void(Scene *scene, sol::state_view,
                                        Entity::id_type, sol::object)>>
      m_EcsAdders;
};
}  // namespace ashfault

#endif

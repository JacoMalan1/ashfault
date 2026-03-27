#include <ashfault/core/component/light.h>
#include <ashfault/core/component/script.h>
#include <ashfault/core/component/tag.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/event.h>
#include <ashfault/core/event/script_attach.h>
#include <ashfault/core/event/scene_start.h>
#include <ashfault/core/layer/script_layer.hpp>
#include <ashfault/editor/context.h>
#include <spdlog/spdlog.h>

#include <iterator>
#include <memory>
#include <sol/forward.hpp>
#include <sol/object.hpp>
#include <sol/state_view.hpp>
#include <sol/types.hpp>

namespace ashfault {
ScriptLayer::ScriptLayer(std::shared_ptr<AssetManager> asset_manager,
                         RuntimeContext *context)
    : Layer(),
      m_LuaState(),
      m_AssetManager(asset_manager),
      m_Context(context) {}

void ScriptLayer::on_attach(LayerStack *) {
  m_ScriptLogger = spdlog::default_logger()->clone("script");
  bind_engine_functions();
}

void ScriptLayer::on_detach() {}

void ScriptLayer::on_update(float dt) {
  if (m_Context->active_scene) {
    for (auto &e : m_Context->active_scene->entities()) {
      auto script = m_Context->active_scene->component_registry()
                        .get_component<ScriptComponent>(e);
      if (script.has_value()) {
        if (!script.value()->script.get()->is_initialized()) {
          script.value()->script.get()->init(m_LuaState);
        }
        script.value()->script.get()->on_update(dt, e);
      }
    }
  }
}

void ScriptLayer::on_event(Event &event) {
  Dispatcher dispatcher;
  dispatcher.dispatch<ScriptAttachEvent>(event, [&](ScriptAttachEvent &ev) {
    auto script =
        m_AssetManager->load<Script>(ev.script_path(), ev.script_path());
    script.get()->init(m_LuaState);
    ScriptComponent component{.script = script};
    m_Context->active_scene->component_registry().add_component(ev.entity(),
                                                                component);
  });

  dispatcher.dispatch<SceneStartEvent>(event, [&](SceneStartEvent &ev) {
    auto *scene = ev.scene();
    for (auto e : scene->entities()) {
      auto script =
          scene->component_registry().get_component<ScriptComponent>(e);
      if (script.has_value()) {
        if (!script.value()->script.get()->is_initialized()) {
          script.value()->script.get()->init(m_LuaState);
        }
        script.value()->script.get()->on_scene_start(e);
      }
    }
  });
}

void ScriptLayer::bind_engine_functions() {
  m_LuaState.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math,
                            sol::lib::string);
  m_LuaState.set_function(
      "print", [&](const std::string &s) { m_ScriptLogger->info(s); });
  m_SceneTable = m_LuaState.create_named_table("Scene");

  m_LuaState.new_usertype<glm::vec3>(
      "Vec3", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
      "x", &glm::vec3::x, "y", &glm::vec3::y, "z", &glm::vec3::z);
  m_LuaState.new_usertype<TransformComponent>(
      "Transform", sol::constructors<TransformComponent()>(), "position",
      &TransformComponent::position, "rotation", &TransformComponent::rotation,
      "scale", &TransformComponent::scale);
  m_LuaState.new_usertype<TagComponent>(
      "Tag", sol::constructors<TagComponent()>(), "tag", &TagComponent::tag);
  m_LuaState.new_usertype<DirectionalLightComponent>(
      "DirectionalLight", sol::constructors<DirectionalLightComponent()>(),
      "direction", &DirectionalLightComponent::direction, "color",
      &DirectionalLightComponent::color);
  m_LuaState.new_usertype<PointLightComponent>(
      "PointLight", sol::constructors<PointLightComponent()>(),
      "position", &PointLightComponent::position, "color",
      &PointLightComponent::color);

  register_ecs_component<TransformComponent>("Transform");
  register_ecs_component<TagComponent>("Tag");
  register_ecs_component<DirectionalLightComponent>("DirectionalLight");
  register_ecs_component<PointLightComponent>("PointLight");
  m_SceneTable.set_function(
      "GetComponent",
      [&](sol::this_state ts, Entity::id_type e,
          sol::table type_table) -> sol::object {
        sol::state_view lua(ts);
        auto *key = type_table.pointer();
        auto it = m_EcsGetters.find(key);
        if (it == m_EcsGetters.end() || !m_Context->active_scene)
          return sol::nil;
        return it->second(m_Context->active_scene, lua, e);
      });
  m_SceneTable.set_function(
      "AddComponent", [&](sol::this_state ts, Entity::id_type e,
                          sol::table type_table, sol::object component) {
        sol::state_view lua(ts);
        auto *key = type_table.pointer();
        auto it = m_EcsAdders.find(key);
        if (it == m_EcsAdders.end() || !m_Context->active_scene) return;
        it->second(m_Context->active_scene, lua, e, component);
      });
  m_SceneTable.set_function(
      "GetEntities", [&](sol::this_state ts) -> sol::object {
        sol::state_view lua(ts);
        if (m_Context->active_scene) {
          std::vector<Entity::id_type> entities{};
          entities.reserve(m_Context->active_scene->entities().size());
          std::transform(m_Context->active_scene->entities().begin(),
                         m_Context->active_scene->entities().end(),
                         std::back_inserter(entities),
                         [](Entity e) { return e.handle(); });
          return sol::make_object(lua, entities);
        }
        return sol::make_object(lua, sol::nil);
      });
  m_SceneTable.set_function(
      "CreateEntity", [&](sol::this_state ts) -> sol::object {
        sol::state_view lua(ts);
        return m_Context->active_scene
                   ? sol::make_object(lua,
                                      m_Context->active_scene->create_entity())
                   : sol::make_object(lua, sol::nil);
      });
}
}  // namespace ashfault

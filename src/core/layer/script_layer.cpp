#include <ashfault/core/component/script.h>
#include <ashfault/core/component/tag.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/event.h>
#include <ashfault/core/event/script_attach.h>
#include <ashfault/core/layer/script_layer.hpp>
#include <ashfault/editor/context.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <sol/forward.hpp>

namespace ashfault {
ScriptLayer::ScriptLayer(std::shared_ptr<AssetManager> asset_manager,
                         RuntimeContext *context)
    : Layer(),
      m_LuaState(),
      m_AssetManager(asset_manager),
      m_Context(context) {}

void ScriptLayer::on_attach(LayerStack *) {
  m_LuaState.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);
  m_LuaState.set_function(
      "print", [](const std::string &s) { SPDLOG_INFO("SCRIPT: {}", s); });
  m_SceneTable = m_LuaState.create_named_table("Scene");

  sol::usertype<glm::vec3> vec3 =
      m_LuaState.new_usertype<glm::vec<3, float>>("Vec3");
  vec3["x"] = sol::property([](const glm::vec3 &x) { return x.x; },
                            [](glm::vec3 &x, float val) { x.x = val; });
  vec3["y"] = sol::property([](const glm::vec3 &x) { return x.y; },
                            [](glm::vec3 &x, float val) { x.y = val; });
  vec3["z"] = sol::property([](const glm::vec3 &x) { return x.z; },
                            [](glm::vec3 &x, float val) { x.z = val; });

  sol::usertype<TransformComponent> transform =
      m_LuaState.new_usertype<TransformComponent>("Transform");
  transform["position"] = sol::property(
      [](const TransformComponent &x) { return x.position; },
      [](TransformComponent &x, glm::vec3 val) { x.position = val; });
  transform["rotation"] = sol::property(
      [](const TransformComponent &x) { return x.rotation; },
      [](TransformComponent &x, glm::vec3 val) { x.rotation = val; });
  transform["scale"] = sol::property(
      [](const TransformComponent &x) { return x.scale; },
      [](TransformComponent &x, glm::vec3 val) { x.scale = val; });

  sol::usertype<TagComponent> tag =
      m_LuaState.new_usertype<TagComponent>("Tag");
  tag["tag"] =
      sol::property([](const TagComponent &x) { return x.tag; },
                    [](TagComponent &x, std::string val) { x.tag = val; });

  register_ecs_component<TransformComponent>("Transform");
  register_ecs_component<TagComponent>("Tag");
  m_SceneTable.set_function(
      "GetComponent",
      [&](sol::this_state ts, Entity::id_type e,
          sol::table type_table) -> sol::object {
        sol::state_view lua(ts);
        auto *key = type_table.pointer();
        auto it = m_EcsMappings.find(key);
        if (it == m_EcsMappings.end() || !m_Context->active_scene)
          return sol::nil;
        return it->second(m_Context->active_scene, lua, e);
      });
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
}

void ScriptLayer::bind_engine_functions() {}
}  // namespace ashfault

#include <ashfault/core/component/light.h>
#include <ashfault/core/component/script.h>
#include <ashfault/core/component/tag.h>
#include <ashfault/core/component/transform.h>
#include <ashfault/core/input.h>
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
  m_InputTable = m_LuaState.create_named_table("Input");
  m_KeyTable = m_LuaState.create_named_table("Key");

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
      "PointLight", sol::constructors<PointLightComponent()>(), "position",
      &PointLightComponent::position, "color", &PointLightComponent::color);

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
  m_InputTable.set_function("RegisterAction",
                            [&](const std::string &action_name) {
                              return Input::register_action(action_name);
                            });
  m_InputTable.set_function("GetActionId", [&](const std::string &action_name) {
    return Input::get_action_id(action_name);
  });
  m_InputTable.set_function(
      "GetAction", [&](sol::this_state ts, sol::object action) -> sol::object {
        if (action.get_type() == sol::type::string) {
          return sol::make_object(ts,
                                  Input::get_action(action.as<std::string>()));
        } else if (action.get_type() == sol::type::number) {
          return sol::make_object(ts, Input::get_action(action.as<ActionId>()));
        } else {
          m_ScriptLogger->error(
              "Incorrect input type for function GetAction: should be a string "
              "or integer");
          return sol::make_object(ts, sol::nil);
        }
      });
  m_InputTable.set_function("BindAction", [&](sol::object action,
                                              std::vector<int> keys) {
    KeyCombination combination;
    combination.keys.reserve(keys.size());
    for (int k : keys) {
      combination.keys.push_back(static_cast<Key>(k));
    }
    if (action.get_type() == sol::type::string) {
      Input::bind_action(action.as<std::string>(), {combination});
    } else if (action.get_type() == sol::type::number) {
      Input::bind_action(action.as<ActionId>(), {combination});
    } else {
      m_ScriptLogger->error(
          "Incorrect input type for function BindAction: should be a string "
          "or integer");
    }
  });

  m_KeyTable["Unknown"] = static_cast<int>(Key::Unknown);
  m_KeyTable["Space"] = static_cast<int>(Key::Space);
  m_KeyTable["Apostrophe"] = static_cast<int>(Key::Apostrophe);
  m_KeyTable["Comma"] = static_cast<int>(Key::Comma);
  m_KeyTable["Minus"] = static_cast<int>(Key::Minus);
  m_KeyTable["Period"] = static_cast<int>(Key::Period);
  m_KeyTable["Slash"] = static_cast<int>(Key::Slash);

  m_KeyTable["Num0"] = static_cast<int>(Key::Num0);
  m_KeyTable["Num1"] = static_cast<int>(Key::Num1);
  m_KeyTable["Num2"] = static_cast<int>(Key::Num2);
  m_KeyTable["Num3"] = static_cast<int>(Key::Num3);
  m_KeyTable["Num4"] = static_cast<int>(Key::Num4);
  m_KeyTable["Num5"] = static_cast<int>(Key::Num5);
  m_KeyTable["Num6"] = static_cast<int>(Key::Num6);
  m_KeyTable["Num7"] = static_cast<int>(Key::Num7);
  m_KeyTable["Num8"] = static_cast<int>(Key::Num8);
  m_KeyTable["Num9"] = static_cast<int>(Key::Num9);

  m_KeyTable["Semicolon"] = static_cast<int>(Key::Semicolon);
  m_KeyTable["Equal"] = static_cast<int>(Key::Equal);

  m_KeyTable["A"] = static_cast<int>(Key::A);
  m_KeyTable["B"] = static_cast<int>(Key::B);
  m_KeyTable["C"] = static_cast<int>(Key::C);
  m_KeyTable["D"] = static_cast<int>(Key::D);
  m_KeyTable["E"] = static_cast<int>(Key::E);
  m_KeyTable["F"] = static_cast<int>(Key::F);
  m_KeyTable["G"] = static_cast<int>(Key::G);
  m_KeyTable["H"] = static_cast<int>(Key::H);
  m_KeyTable["I"] = static_cast<int>(Key::I);
  m_KeyTable["J"] = static_cast<int>(Key::J);
  m_KeyTable["K"] = static_cast<int>(Key::K);
  m_KeyTable["L"] = static_cast<int>(Key::L);
  m_KeyTable["M"] = static_cast<int>(Key::M);
  m_KeyTable["N"] = static_cast<int>(Key::N);
  m_KeyTable["O"] = static_cast<int>(Key::O);
  m_KeyTable["P"] = static_cast<int>(Key::P);
  m_KeyTable["Q"] = static_cast<int>(Key::Q);
  m_KeyTable["R"] = static_cast<int>(Key::R);
  m_KeyTable["S"] = static_cast<int>(Key::S);
  m_KeyTable["T"] = static_cast<int>(Key::T);
  m_KeyTable["U"] = static_cast<int>(Key::U);
  m_KeyTable["V"] = static_cast<int>(Key::V);
  m_KeyTable["W"] = static_cast<int>(Key::W);
  m_KeyTable["X"] = static_cast<int>(Key::X);
  m_KeyTable["Y"] = static_cast<int>(Key::Y);
  m_KeyTable["Z"] = static_cast<int>(Key::Z);

  m_KeyTable["LeftBracket"] = static_cast<int>(Key::LeftBracket);
  m_KeyTable["Backslash"] = static_cast<int>(Key::Backslash);
  m_KeyTable["RightBracket"] = static_cast<int>(Key::RightBracket);
  m_KeyTable["GraveAccent"] = static_cast<int>(Key::GraveAccent);
  m_KeyTable["World1"] = static_cast<int>(Key::World1);
  m_KeyTable["World2"] = static_cast<int>(Key::World2);

  m_KeyTable["Escape"] = static_cast<int>(Key::Escape);
  m_KeyTable["Enter"] = static_cast<int>(Key::Enter);
  m_KeyTable["Tab"] = static_cast<int>(Key::Tab);
  m_KeyTable["Backspace"] = static_cast<int>(Key::Backspace);
  m_KeyTable["Insert"] = static_cast<int>(Key::Insert);
  m_KeyTable["Delete"] = static_cast<int>(Key::Delete);
  m_KeyTable["Right"] = static_cast<int>(Key::Right);
  m_KeyTable["Left"] = static_cast<int>(Key::Left);
  m_KeyTable["Down"] = static_cast<int>(Key::Down);
  m_KeyTable["Up"] = static_cast<int>(Key::Up);

  m_KeyTable["PageUp"] = static_cast<int>(Key::PageUp);
  m_KeyTable["PageDown"] = static_cast<int>(Key::PageDown);
  m_KeyTable["Home"] = static_cast<int>(Key::Home);
  m_KeyTable["End"] = static_cast<int>(Key::End);

  m_KeyTable["CapsLock"] = static_cast<int>(Key::CapsLock);
  m_KeyTable["ScrollLock"] = static_cast<int>(Key::ScrollLock);
  m_KeyTable["NumLock"] = static_cast<int>(Key::NumLock);
  m_KeyTable["PrintScreen"] = static_cast<int>(Key::PrintScreen);
  m_KeyTable["Pause"] = static_cast<int>(Key::Pause);

  m_KeyTable["F1"] = static_cast<int>(Key::F1);
  m_KeyTable["F2"] = static_cast<int>(Key::F2);
  m_KeyTable["F3"] = static_cast<int>(Key::F3);
  m_KeyTable["F4"] = static_cast<int>(Key::F4);
  m_KeyTable["F5"] = static_cast<int>(Key::F5);
  m_KeyTable["F6"] = static_cast<int>(Key::F6);
  m_KeyTable["F7"] = static_cast<int>(Key::F7);
  m_KeyTable["F8"] = static_cast<int>(Key::F8);
  m_KeyTable["F9"] = static_cast<int>(Key::F9);
  m_KeyTable["F10"] = static_cast<int>(Key::F10);
  m_KeyTable["F11"] = static_cast<int>(Key::F11);
  m_KeyTable["F12"] = static_cast<int>(Key::F12);
  m_KeyTable["F13"] = static_cast<int>(Key::F13);
  m_KeyTable["F14"] = static_cast<int>(Key::F14);
  m_KeyTable["F15"] = static_cast<int>(Key::F15);
  m_KeyTable["F16"] = static_cast<int>(Key::F16);
  m_KeyTable["F17"] = static_cast<int>(Key::F17);
  m_KeyTable["F18"] = static_cast<int>(Key::F18);
  m_KeyTable["F19"] = static_cast<int>(Key::F19);
  m_KeyTable["F20"] = static_cast<int>(Key::F20);
  m_KeyTable["F21"] = static_cast<int>(Key::F21);
  m_KeyTable["F22"] = static_cast<int>(Key::F22);
  m_KeyTable["F23"] = static_cast<int>(Key::F23);
  m_KeyTable["F24"] = static_cast<int>(Key::F24);
  m_KeyTable["F25"] = static_cast<int>(Key::F25);

  m_KeyTable["KP0"] = static_cast<int>(Key::KP0);
  m_KeyTable["KP1"] = static_cast<int>(Key::KP1);
  m_KeyTable["KP2"] = static_cast<int>(Key::KP2);
  m_KeyTable["KP3"] = static_cast<int>(Key::KP3);
  m_KeyTable["KP4"] = static_cast<int>(Key::KP4);
  m_KeyTable["KP5"] = static_cast<int>(Key::KP5);
  m_KeyTable["KP6"] = static_cast<int>(Key::KP6);
  m_KeyTable["KP7"] = static_cast<int>(Key::KP7);
  m_KeyTable["KP8"] = static_cast<int>(Key::KP8);
  m_KeyTable["KP9"] = static_cast<int>(Key::KP9);
  m_KeyTable["KPDecimal"] = static_cast<int>(Key::KPDecimal);
  m_KeyTable["KPDivide"] = static_cast<int>(Key::KPDivide);
  m_KeyTable["KPMultiply"] = static_cast<int>(Key::KPMultiply);
  m_KeyTable["KPSubtract"] = static_cast<int>(Key::KPSubtract);
  m_KeyTable["KPAdd"] = static_cast<int>(Key::KPAdd);
  m_KeyTable["KPEnter"] = static_cast<int>(Key::KPEnter);
  m_KeyTable["KPEqual"] = static_cast<int>(Key::KPEqual);

  m_KeyTable["LeftShift"] = static_cast<int>(Key::LeftShift);
  m_KeyTable["LeftControl"] = static_cast<int>(Key::LeftControl);
  m_KeyTable["LeftAlt"] = static_cast<int>(Key::LeftAlt);
  m_KeyTable["LeftSuper"] = static_cast<int>(Key::LeftSuper);
  m_KeyTable["RightShift"] = static_cast<int>(Key::RightShift);
  m_KeyTable["RightControl"] = static_cast<int>(Key::RightControl);
  m_KeyTable["RightAlt"] = static_cast<int>(Key::RightAlt);
  m_KeyTable["RightSuper"] = static_cast<int>(Key::RightSuper);
  m_KeyTable["Menu"] = static_cast<int>(Key::Menu);

  m_KeyTable["Mouse1"] = static_cast<int>(Key::Mouse1);
  m_KeyTable["Mouse2"] = static_cast<int>(Key::Mouse2);
  m_KeyTable["Mouse3"] = static_cast<int>(Key::Mouse3);
  m_KeyTable["Mouse4"] = static_cast<int>(Key::Mouse4);
  m_KeyTable["Mouse5"] = static_cast<int>(Key::Mouse5);
  m_KeyTable["Mouse6"] = static_cast<int>(Key::Mouse6);
  m_KeyTable["Mouse7"] = static_cast<int>(Key::Mouse7);
  m_KeyTable["Mouse8"] = static_cast<int>(Key::Mouse8);
  m_KeyTable["MouseLeft"] = static_cast<int>(Key::MouseLeft);
  m_KeyTable["MouseRight"] = static_cast<int>(Key::MouseRight);
  m_KeyTable["MouseMiddle"] = static_cast<int>(Key::MouseMiddle);
}
}  // namespace ashfault
